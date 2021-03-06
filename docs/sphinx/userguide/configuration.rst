.. _configuration:

===============
 Configuration
===============

The QA would run basically on the command-line only with the specification of
the target(s) to be checked. However, using options and gathering these
in configure files facilitates checks.

Configuration options follow a specific syntax with option names.

.. code-block:: bash

  KEY-WORD                             enable key-word; equivalent to key-word=t
  KEY-WORD   =     value[,value ...]   assign key-word a [comma-separated] value; overwrite
  KEY-WORD   +=    value[,value ...]   same, but appended


Partitioning of Data Sets
-------------------------

The specification of a path to a data directory tree by option ``PROJECT_DATA``
results in a check of every NetCDF files found within the directory-tree. This can be further customised by key-words ``SELECT`` and ``LOCK``, which follow special rules.

.. code-block:: bash

  KEY-WORD                            var1[,var2,...]   specified variables for every path; += mode
  KEY-WORD      path1[,path2,...] [=]                   all variables within the specified path; +=mode
  KEY-WORD      path1[,path2,...]  =  [var1[,var2,...]  specified variables within the given paths; += mode
  KEY-WORD  =   path1[,path2,...]  =  [var1[,var2,...]  same, but overwrite mode
  KEY-WORD  +=  path1[,path2,...]  =  [var1[,var2,...]  append to a previous assignment



Some options act on other options:

* Option names on the command-line are case-insensitive, they have to be prefixed by '-e' or '-E' (a blank or _ may follow).

* Highest precedence is for options on the command-line.

* If path has no leading '/', then the search is relative to the path specified by option PROJECT_DATA.

* Special for options ``-S arg`` or an appended path/filename on the command-line: cancellation of previous SELECTions.

* If SELECTions are specified on the command-line (options -S) with an absolute path, i.e. beginning with '/', then PROJECT_DATA specified in any config-file is cancelled.

* All SELECTions refer to atomic variables, i.e. all sub-temporal files.

* LOCKing gets the higher precedence over SELECTion.

* SELECT: Path and variable items are separated by the '=' character, which may be omitted when there is no variable (except the case that the path item contains no '/' character).

* Regular expressions may be applied to both path(s) and variable(s).

* A particular variable is selected if the beginning of the name is unambiguous, e.g. specifying 'tas' would select all tas, tasmax, and tasmin files. Note that every file name begins with ``variable_...`` for CMIP5/CORDEX, thus, use ``tas_`` for this alone.

* Former option names are valid, if they have changed in the meantime.

Configuration Files
===================

A description of the configuration options is given in the QA_DKRZ root path.

.. code-block:: bash

    package-path/tables/projects/"project-name"/"project-name"_qa.conf

Multiple configuration files and options may be specified following a given
precedence. This facilitates having a file with short-term options (in a
file provided by the -f option on the command-line), another one with settings
to site-specific demands, which are robust against changes in the repository,
and long-term default settings from the repository.
A sequence of configuration files is defined by ``QA_CONF=conf-file``
assignments embedded in the configuration files.
The precedence of configuration files/options is given below from highest to
lowest:

-  directly on the command-line
-  in the task-file (``-f file``) specified on the command-line.
-  QA_CONF assignments embedded (descending starts from the ``-f file``).
-  site-specific files provided by files located straight in
   ``/package-path/tables``.
-  defaults for the entire project:

All options may also be provided on the command-line plus some more for testing, see
``/package-path/QA-DKRZ/scripts/qa_DKRZ --help``.

Experiment Names
================

QA-DKRZ checks files individually. The results are contained in files
unique for a specific scope. Albeit most projects have defined a term
'experiment', this is not suitable to provide a realm common to a sub-set
of data files.

For CMIP5/6 and CORDEX, an unambiguous scope is defined by the properties of
the so-called Data Reference System (DRS), i.e. the components of the path to
the variables. The options ``LOG_PATH_INDEX`` and ``DRS_PATH_BASE``
define a unique experiment-name, where the former contains a comma-separated list
of indices of the path components and the latter the starting point with
index=0, e.g. ``DRS_PATH_BASE=output`` and ``LOG_PATH_INDEX=1,2,3,4,6``.
An example is given in :ref:`results`.

Similar is for building the name of a consistency table, which is used to check
consistency between a parent and its child experiment as well as between the temporal sequence within an atomic variable.

.. note:: If ``CT_PATH_INDEX`` is not set, then consistency checks are disabled.
