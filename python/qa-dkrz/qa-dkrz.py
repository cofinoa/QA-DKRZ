
'''
Created on 17.03.2016

@author: hdh
'''

import sys
import os
import glob
import re
import shutil
import subprocess
import copy
from   types import *

import qa_util
import qa_init
import qa_version

from Queue import Queue
from threading import Thread

from qa_config import QaConfig
#from qa_config import CfgFile
from qa_log import Log
from qa_util import GetPaths
from qa_launcher import QaLauncher
from qa_exec import QaExec
from qa_summary import LogSummary

# struct like classes containing variables
class GlobalVariables(object):
    # valid across the total run
    pass
class ThreadVariables(object):
    # determined at every get_next_variable() call
    pass


def clear(qa_var_path, fBase, logfile):
    if qaConf.isOpt('CLEAR_LOGFILE'):
        try:
            g_vars.clear_fBase
        except:
            g_vars.clear_fBase = []

        # mark for clearance of logfiles if enabled,
        # note that index() finds always a valid one.
        ix = g_vars.log_fnames.index(t_vars.log_fname)
        if ix == len( g_vars.clear_fBase):
            g_vars.clear_fBase.append([])

        g_vars.clear_fBase[ix].append(fBase)

    # symbolic link algorithm is skipped

    # clearing section for regular files
    fs=glob.glob(os.path.join(qa_var_path, '*_' + fBase + '*' ) )

    if os.path.exists( os.path.join(qa_var_path, 'core')):
        fs.append('core')

    # remove selected
    for f in fs:
        try:
            os.remove(f)
        except OSError:
            pass

    clearLog(qa_var_path, fBase, logfile)

    return


def clearLog(qa_var_path, fBase, logfile):
   call='grep -n "[[:space:]]*file:[[:space:]]*' + fBase + '" ' + logfile + ' 2> /dev/null'
   try:
      check_output = subprocess.check_output(call, shell=True)
   except:
      return
   else:
      words=check_output.split()

   nums=[]
   for i in range( len(words[::2])):
      nums.append( words[i][:-1] )

   call='grep -n "[[:space:]]*data-set:[[:space:]]*'+fBase+'" '+ logfile + ' 2> /dev/null'
   try:
      check_output = subprocess.check_output(call, shell=True)
   except:
      return
   else:
      words=check_output.split()

   for i in range( len(words[::2])):
      nums.append( words[i][:-1] )

   nums = sorted(nums, reverse=True)

   for i in range( len(nums)):
      num = nums[i] - 1
      call='sed -i "/' + str(num) + '/,/status:/ d" ' + logfile

   return


def clearInq(qa_var_path, fBase, logfile):
    if not qaConf.isOpt('CLEAR'):
       return

#    isFollowLink = False
    v_clear = qaConf.getOpt('CLEAR')

    if v_clear == 't':
        clear(qa_var_path, fBase, logfile)
        return

    isClear=False

    for f in v_clear.split(','):
        if f == 't':
            break  # unconditional

        '''
        if f == 'follow_links':
            if not getOpt.isOpt('DEREFERENCE_SYM_LINKS'):

                isFollowLink = True
            isClear = True
        el'''

        if f == 'only':
            isClear = True
        elif f == 'lock':
            # locked  files
            if len( glob.glob(os.path.join(qa_var_path, 'qa_lock_' + fBase +'*') ) ):
                isClear = True
        elif f == 'note':
            if len( glob.glob(os.path.join(qa_var_path, 'qa_note_' + fBase +'*') ) ):
                isClear = True
        elif qa_util.rstrip(f, sep='=') == 'level':
            # clear specified level
            f=qa_util.lstrip(f, sep='=') + '-'
        elif qa_util.rstrip(f, '=') == 'tag':
            call='sed -n "/' + fBase + '/,/status:/ p" ' + logfile
            try:
                check_output = subprocess.check_output(call, shell=True)
            except:
                pass
            else:
                requ_tag = qa_util.lstrip(f, '=')

                words=check_output.split()
                sz=len(words)
                for i in range(sz-1):
                    w1 = qa_util.lstrip(words[i], '=')
                    l_w1=len(w1)
                    l_tg=len(requ_tag)
                    if words[i] == 'tag:' and w1 == requ_tag:
                        isClear=True
                        break


            '''
            # e.g. 'L1-${tag}: where tag=CF_12 would match CF_12, CF_12x etc.
            f='[\w]*-*' + qa_util.lstrip(f, sep='=') + '.*: '

            fls = glob.glob(os.path.join(qa_var_path, 'qa_note_' + fBase +'*') )
            fls.extend(glob.glob(os.path.join(qa_var_path, 'qa_lock_' + fBase +'*') ))

            for fl in fls:
                try:
                    subprocess.check_call('grep', '-q', f, fl)
                except:
                    pass
                else:
                    isClear = True
                    break
            '''
        else:
            # CLEAR=var=name
            pos = f.find('var=')
            if pos > -1:
                f = f[pos+4:]

            # else:
                # CLEAR=varName

            regExp = re.match(f, fBase)
            if regExp:
                tmp_ls = glob.glob(os.path.join(qa_var_path, '*_' + fBase + '*'))
                if len(tmp_ls):
                    isClear = True

        if isClear:
            # now do the clearance
            clear(qa_var_path, fBase, logfile)
            return

    return


def final():
   # special for no project, but a single file: display log-file
   if not qaConf.isOpt("PROJECT_DATA"):
      for logName in g_vars.log_fnames:
         logName = os.path.join(qaConf.dOpts["QA_RESULTS"], 'check_logs', logName+'.log')

         with open(logName, 'r') as f:
            print f.read()

      shutil.rmtree(qaConf.dOpts["QA_RESULTS"])

      return

   summary()

   qaConf.cfg.write_file()

   return


def get_all_logfiles():
    f_logs=[]
    if qaConf.isOpt('SHOW_EXP'):
        isShowExp=True
        lfn_var={}

    while True:
        try:
            # fBase: list of filename bases corresponding to variables;
            #          usually a single one. F
            # fNames: corresponding sub-temporal files
            data_path, fBase, fNames = getPaths.next()

        except StopIteration:
            break
        else:
            sub_path = data_path[g_vars.prj_dp_len+1:]
            f_log = qa_util.get_experiment_name(g_vars, qaConf, fB=fBase,
                                            sp=sub_path)
            if isShowExp:
                if f_log in lfn_var:
                    if not sub_path in lfn_var[f_log]:
                        lfn_var[f_log].append(sub_path)
                else:
                    lfn_var[f_log] = [ sub_path ]

            f_logs.append( f_log )

    if isShowExp:
        for key in lfn_var:
            print '\nLogfile-name: ' + key

            for p in lfn_var[key]:
                print '   Variable: ' + p

        sys.exit(0)

    return f_logs


def get_next_variable(data_path, fBase, fNames):

    # test for parameters QUERY_..., if set at all
    if query_files(qaConf, g_vars.validNcPrg, data_path, fNames):
       return []

    t_vars.sub_path = data_path[g_vars.prj_dp_len+1:]
    t_vars.var_path = os.path.join(g_vars.res_dir_path, 'data',
                                    t_vars.sub_path )

    # any QA result file of a previous check?
    t_vars.qaFN = 'qa_' + fBase + '.nc'
    qaNc = os.path.join(t_vars.var_path, t_vars.qaFN)

    # experiment_name
    t_vars.log_fname = \
        qa_util.get_experiment_name(g_vars, qaConf, fB=fBase,
                                    sp=t_vars.sub_path)
    t_vars.consistency_tname = \
        qa_util.get_project_table_name(g_vars, qaConf, fB=fBase,
                                       sp=t_vars.sub_path)

    # Any locks? Any clearance of previous results?
    if testLock(t_vars, fBase):
        return []

    syncCall = g_vars.syncFilePrg + ' --only-marked'

    if qaConf.isOpt('TEST_FNAME_ALIGNMENT'):
        syncCall += ' --fname-alignment --continuous'

    if os.path.exists(qaNc):
        syncCall += ' -p ' + qaNc
    else:
        qaNc=''

    if qaConf.isOpt('TIME_LIMIT'):
        syncCall += ' -l ' + qaConf.getOpt('TIME_LIMIT')

    syncCall += ' -P ' + data_path

    for f in fNames:
        syncCall += ' ' + f

    try:
        next_file_str = subprocess.check_output(syncCall, shell=True)

    except subprocess.CalledProcessError as e:
        status = e.returncode

        if status == 1:
            return []

        info = e.output.splitlines()
        isLog=False

        if status == 4:
            # a fixed variable?
            if len(qaNc):
               # previously processed
               return []
            elif len(fNames) == 1:
               next_file_str = fNames[0]

        elif status == 3 or status == 5 or status == 6:
            g_vars.anyProgress = True

            if status == 3:
                annot='Unspecific failure across sub-temporal file sequence'
                tag='M5'
                concl='DRS(F): FAIL'
            elif status == 5:
                annot='File(s) with invalid time data'
                tag='M9'
                concl='TIME: FAIL'
                info=[]
            elif status == 6:
                annot='Invalid NetCDF file'
                tag='M0'
                concl='CF: FAIL, CV: FAIL, DATA: FAIL, DRS: PASS, TIME: FAIL'
                info=[]

            entry_id = log.append(t_vars.log_fname, f=fBase, d_path=data_path,
                       r_path=g_vars.res_dir_path, sub_path=t_vars.sub_path,
                       annotation=annot,
                       impact='L2', tag=tag, info=info,
                       conclusion=concl, status=status)
            isLog = True

        elif status > 8:
            g_vars.anyProgress = True

            entry_id = log.append( t_vars.log_fname, f=fBase, d_path=data_path,
                        r_path=g_vars.res_dir_path, sub_path=t_vars.sub_path,
                        annotation='ambiguous sub-temporal file sequence',
                        impact='L2', tag='M5', info=info,
                        conclusion='DRS(F): FAIL', status=status)
            isLog = True

        if isLog:
            log.write_entry(entry_id,
                            g_vars.check_logs_path,
                            t_vars.log_fname)

            return []

    try:
        lst = next_file_str.splitlines()
    except:
        lst=[]

    if qaConf.isOpt('TEST_FNAME_ALIGNMENT'):
        lst=[]

    return lst


def get_version(qaConf):

    # this is mandatory
    isVerbose=False  # directly from the tables

    if qaConf.isOpt('VERBOSE'):
        isVerbose=True
    elif qaConf.isOpt("QA_VERSION"):
        rev=qaConf.dOpts["QA_VERSION"]
        rev += '|' + qaConf.dOpts["CF_VERSION"]

        prj = qaConf.getOpt('PROJECT')
        if qaConf.isOpt(prj + '_VERSION'):
            rev += '|' + qaConf.dOpts[prj + '_VERSION']
        else:
            isVerbose=True
    else:
        print 'Incomplete installation.'
        print 'Please, run: qa-dkrz install --up --force PROJECT-name'

        sys.exit(1)

    if isVerbose:
        com_line_opts = ["--section=" + qaConf.getOpt("QA_SRC")]

        try:
            rev = qa_version.get_version( opts=qaConf.dOpts,
                                        com_line_opts=com_line_opts)
        except:
            print 'please, run: qa-dkrz install --force --up ' + prj
            sys.exit(1)

    if qaConf.isOpt('SHOW_VERSION'):
        print rev
        sys.exit(0)

    qaConf.dOpts["QA_VERSION"] = rev

    return


def prepareExample(qaConf):
    if not qaConf.isOpt('PROJECT'):
        qaConf.addOpt("PROJECT", 'CORDEX')

    if qaConf.isOpt('WORK'):
        currdir=qaConf.getOpt('WORK')
    else:
        currdir=qaConf.getOpt('CURR_DIR')

    #currdir=os.path.join(currdir, 'example')
    qaConf.dOpts['QA_RESULTS'] = os.path.join(currdir, 'results')

    if not qa_util.mkdirP(currdir):
        sys.exit(1)

    os.chdir(currdir)
    qa_util.rm_r( 'results', 'config.txt', 'qa-test.task' )

    print 'make examples in ' + currdir
    print 'make qa_test.task'

    taskFile = os.path.join(QA_SRC, 'example', 'templates', 'qa-test.task')
    shutil.copy( taskFile, currdir)
    taskFile = 'qa-test.task'

    # replace templates within QA_SRC/example
    sub=[]
    repl=[]

    sub.append('PROJECT_DATA=data')
    repl.append('PROJECT_DATA=' + os.path.join(currdir, 'data') )

    sub.append('QA_RESULTS=results')
    repl.append('QA_RESULTS=' + os.path.join(currdir, 'results') )

    qa_util.f_str_replace(taskFile, sub, repl)

    if not qa_util.which("ncgen"):
        print "building data in example requires the ncgen utility"
        sys.exit(1)

    if not os.path.isdir( os.path.join(currdir, 'data') ):
        # data
        print 'make data'
        p=os.path.join(QA_SRC, 'example', 'templates', 'data.tbz')
        subprocess.call(["tar", "--bzip2", "-xf", p ])

        for rs, ds, fs in os.walk('data'):
            for f in fs:
                nc_f = f[:len(f)-3] + 'nc'
                t_f=os.path.join(rs, f)
                t_nc=os.path.join(rs, nc_f)
                try:
                    subprocess.call(["ncgen", "-k", "3", "-o", t_nc, t_f])
                except:
                    print 'making of example failed'
                    sys.exit(1)
                else:
                    qa_util.rm_r(t_f)

    print 'run' + sys.argv[0] + " -m -f " + currdir + "/qa-test.task"

    qaConf = QaConfig(QA_SRC, ['-m', '-f', currdir + "/qa-test.task" ])

    return qaConf


def query_files(qaConf, validNcPrg, p, fs):
    retVal=False

    # QUERY_NON_NC_FILE
    # QUERY_EMPTY_DIR
    # QUERY_EMPTY_FILE
    # QUERY_ONLY_NC
    is_non_nc=False
    is_empty_dir=False
    is_empty_file=False

    if len(fs) == 0:
       retVal=True
       if qaConf.isOpt('QUERY_EMPTY_DIR'):
          pass
    else:
       for f in fs:
         f = os.path.join(p, f)

         if os.path.getsize(f) == 0:
            if qaConf.isOpt('QUERY_EMPTY_FILE'):
               retVal=True
               is_empty_file=True
         elif f[-3:] == '.nc' or f[-4:] == '.nc4':
            pass
         else:
            validNcPrg += ' ' +  f

            try:
               subprocess.check_call(validNcPrg, shell=True)
            except subprocess.CalledProcessError as e:
               if qaConf.isOpt('QUERY_NON_NC_FILE'):
                  is_non_nc=True

               retVal = True



    return retVal


def run():
    if qaConf.isOpt("SHOW_EXP"):
        f_log = get_all_logfiles()
        sys.exit(0)

    if qaConf.isOpt('SHOW') or qaConf.isOpt('NEXT'):
        g_vars.thread_num = 1

    # the queue is two items longer than the number of threads
    queue = Queue(maxsize=g_vars.thread_num+2)

    launch_list = []

    if g_vars.thread_num < 2:
        # a single thread
        qaExec = QaExec(log, qaConf, g_vars)
        launch_list.append( qaExec )
    else:
        for i in range(g_vars.thread_num):
            launch_list.append( QaLauncher(log, qaConf, g_vars) )

        for i in range(g_vars.thread_num):
            t = Thread(target=launch_list[i].start, args=(queue,))
            t.daemon = True
            t.start()

    is_next_var=False
    if qaConf.isOpt('NEXT_VAR'):
        is_next_var=True
        next_var=qaConf.getOpt('NEXT_VAR')
        count_next_var=0

    while True:

        if qaConf.isOpt('EXPLICIT_FILES'):
           if len(qaConf.dOpts['EXPLICIT_FILES']):
               data_path, f = os.path.split(qaConf.dOpts['EXPLICIT_FILES'][0])
               t_r = qa_util.f_time_range(f)
               fBase = t_r[0]
               fNames = [ f ]
               del qaConf.dOpts['EXPLICIT_FILES'][0]
           else:
               queue.put( ('---EOQ---', '', t_vars), block=True)
               break
        else:
            if is_next_var:
               if count_next_var == next_var:
                  break

               count_next_var += 1

            try:
               # fBase: list of filename bases corresponding to variables;
               #        usually a single one. F
               # fNames: corresponding sub-temporal files
               data_path, fBase, fNames = getPaths.next()

            except StopIteration:
               queue.put( ('---EOQ---', '', t_vars), block=True)
               break
            else:
               isNoPath=False

        # return a list of files which have not been processed, yet.
        # Thus, the list could be empty
        fL = get_next_variable(data_path, fBase, fNames)

        if len(fL) == 0:
           continue

        t_vars.fBase = fBase

        #queue.put( (data_path, fL, t_vars), block=True)
        queue.put( (data_path, fL, copy.deepcopy(t_vars)),
                     block=True)

        if g_vars.thread_num < 2:
            # a single thread
            if not launch_list[0].start(queue,):
                break

    if g_vars.thread_num > 1:
        queue.join()

    if g_vars.thread_num < 2:
        launch_list[0].printStatusLine()
    else:
        launch_list[0].qa_exec.printStatusLine()

    return


def summary():
    # preparation to call a summary object
    if qaConf.isOpt('NO_SUMMARY'):
       return

    if qaConf.isOpt('ONLY_SUMMARY'):
        # build only the summary of previously written log-files.
        f_log = qaConf.getOpt('ONLY_SUMMARY')
        if type(f_log) == StringType:
            f_log = [f_log]

        if type(f_log) != ListType:
            # all the log-files corresponding to the current selection
            f_log = get_all_logfiles()
            f_log.append(g_vars.check_logs_path)
    else:
        f_log = g_vars.log_fnames
        f_log.append(g_vars.check_logs_path)

    # summary object
    log_sum = LogSummary()
    f_log = log_sum.prelude(f_log)
    for i in range( len(f_log) ):
        log_sum.run(f_log[i])

    if qaConf.isOpt('ONLY_SUMMARY'):
        sys.exit(0)

    return


def testLock(t_vars, fBase):
    if qaConf.isOpt('CLEAR'):
        logfile = os.path.join(g_vars.check_logs_path, t_vars.log_fname + '.log')

        clearInq(t_vars.var_path, fBase, logfile)

    # any qa_lock file?
    f_lock = os.path.join(t_vars.var_path, 'qa_lock_' + fBase + '.txt')
    if os.path.exists(f_lock):
        return True

    return False


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.argv.append("--help")

    (isCONDA, QA_SRC) = qa_util.get_QA_SRC(sys.argv[0])

    qa_init.run_install(QA_SRC)  # exit after processing, if any

    # for options on the command-line as well as in configuration files
    qaConf=QaConfig(QA_SRC)

    g_vars = GlobalVariables()
    t_vars = ThreadVariables()

    g_vars.qa_src = QA_SRC
    g_vars.isConda = isCONDA
    g_vars.pid = str(os.getpid())
    g_vars.UDUNITS2_XML_PATH = qaConf.getOpt("UDUNITS2_XML_PATH")

    if 'QA_EXAMPLE' in qaConf.dOpts:
        qaConf = prepareExample(qaConf)

    if isCONDA:
        qaConf.addOpt('CONDA', True)

    log = Log(qaConf.dOpts)

    # obj for getting and iteration next path
    getPaths = GetPaths(qaConf)

    if 'SHOW_CONF' in qaConf.dOpts:
        for opt in qa_util.get_sorted_options(qaConf.dOpts):
            # opt: key=value
            print opt
        sys.exit(0)

    try:
        # get from ~/.qa-dkrz/config.txt; also a check for incomplete installation
        get_version(qaConf)
    except:
        print 'could not run qa-dkrz.get_version()'
        sys.exit(1)

    qa_init.run(log, g_vars, qaConf)

    # the checks
    if not qaConf.getOpt('ONLY_SUMMARY'):
        run()

    final()
