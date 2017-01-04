#include "jsoncpp.cpp"

// #include "qa.h"

CMIP6_CV::CMIP6_CV(QA* p, Variable* v)
{
  pQA = p;
  pGlob = v;
  notes = pQA->notes;

  n_CV = "CV" ;
  cv_path = pQA->tablePath + "/CMIP6_CVs" ;

  cv_table.push_back("activity_id");
  cv_table.push_back("experiment_id");
  cv_table.push_back("frequency");
  cv_table.push_back("grid_label");
  cv_table.push_back("institution_id");
  cv_table.push_back("nominal_resolution");
  cv_table.push_back("realm");
  cv_table.push_back("required_global_attributes");
  cv_table.push_back("source_id");
  cv_table.push_back("source_type");
  cv_table.push_back("table_id");
  cv_table.push_back("mip_era");
}

void
CMIP6_CV::add_requ_glob_att(std::string& id)
{
    if( !hdhC::isAmong(id, vs_requiredGlobalAtts) )
        vs_requiredGlobalAtts.push_back(id);

    return;
}

bool
CMIP6_CV::check_array(std::string& id)
{
   Json::Value* jsnObj;

   if( id == "activity_id")
       jsnObj = &jObj_activity_id ;
   else if( id == "frequency")
       jsnObj = &jObj_frequency ;
   else if( id == "grid_label")
       jsnObj = &jObj_grid_label;
   else if( id == "institution_id")
       jsnObj = &jObj_institution_id;
   else if( id == "nominal_resolution")
       jsnObj = &jObj_nominal_resolution;
   else if( id == "realm")
       jsnObj = &jObj_realm;
   else if( id == "required_global_attributes")
       jsnObj = &jObj_required_global_attributes ;
   else if( id == "source_type")
       jsnObj = &jObj_source_type;
   else if( id == "table_id")
       jsnObj = &jObj_table_id;
   else if( id == "mip_era")
       jsnObj = &jObj_mip_era;
   else
       return false; // no Json obj is related to this global att

   if( jsnObj->isNull() )
       return true; // Missing table? Failed reading?

   const Json::Value jCV =  (*jsnObj)[id];

   // Iterate over the sequence elements.
   std::vector<std::string> vs;
   for( Json::ArrayIndex i=0; i < jCV.size(); ++i )
      vs.push_back(jCV[i].asString()) ;

   // special: is not in the global attributes but provides a list of those
   if( id == "required_global_attributes" )
   {
       vs_requiredGlobalAtts = vs ;
       return true;
   }

   // meta from file
   std::string f_val = pGlob->getAttValue(id);

   if( hdhC::isAmong(f_val, vs) )
     return true;

   // annotation

   return true ;
}

void
CMIP6_CV::check_existence(void)
{
    for( size_t i=0 ; i < vs_requiredGlobalAtts.size() ; ++i )
    {
        if( vs_requiredGlobalAtts[i] == "grid_resolution" )
            vs_requiredGlobalAtts[i] = "nominal_resolution" ;

        if( ! hdhC::isAmong(vs_requiredGlobalAtts[i], pGlob->attName) )
        {
          std::string key("2_1");

          if( notes->inq( key, pQA->fileStr) )
          {
            std::string capt("required ");
            capt += pQA->s_global;
            capt += hdhC::blank;
            capt += hdhC::tf_att();
            capt += "is missing" ;

            (void) notes->operate(capt) ;
            notes->setCheckStatus(n_CV,  pQA->n_fail );
          }

          vs_requiredGlobalAtts[i].clear();
        }
    }
}

void
CMIP6_CV::check_string(std::string& id)
{
   Json::Value* jsnObj;
   bool isJson = true;

   if( id == "activity_id")
       jsnObj = &jObj_activity_id ;
   else if(id == "frequency")
       jsnObj = &jObj_frequency ;
   else if(id == "grid_label")
       jsnObj = &jObj_grid_label;
   else if(id == "institution_id")
       jsnObj = &jObj_institution_id;
   else if(id == "nominal_resolution")
       jsnObj = &jObj_nominal_resolution;
   else if(id == "realm")
       jsnObj = &jObj_realm;
   else if(id == "source_type")
       jsnObj = &jObj_source_type;
   else if(id == "table_id")
       jsnObj = &jObj_table_id;
   else if(id == "mip_era")
       jsnObj = &jObj_mip_era;
   else
       isJson = false; // no Json obj is related to this global att

   if( isJson )
   {
     const Json::Value jCV =  (*jsnObj)[id];

     // meta from file; is also name of jsnObj
     std::string f_inst_id = pGlob->getAttValue(id);
     std::string f_inst = pGlob->getAttValue("institution");

     Json::Value::Members names = jCV.getMemberNames() ;

     std::string str;
     bool is_inst_id=false;
     bool is_inst=false;

     for( size_t i=0 ; i < names.size() ; ++i )
     {
       // checking institution_id
       if( names[i] == f_inst_id )
       {
         is_inst_id=true;
         Json::Value instObj = jCV[f_inst_id] ;

         if( instObj.isString() )
         {
            str = instObj.asString() ;
            if( str == f_inst )
               is_inst=true;

            break;
         }
       }
     }

     // annotation
     if( !is_inst_id )
     {
       std::string key("2_4");

       if( notes->inq( key, n_CV) )
       {
         std::string capt(hdhC::tf_att(hdhC::empty, id, f_inst_id) ) ;
         capt += "does not match global attributes";

         (void) notes->operate(capt) ;
         notes->setCheckStatus(n_CV, pQA->n_fail);
       }

       return;
     }

     if( !is_inst )
     {
       std::string key("2_4");

       if( notes->inq( key, n_CV) )
       {
         std::string capt(hdhC::tf_att(hdhC::empty, "institution", hdhC::colon) ) ;
         capt += "found ";
         capt += hdhC::tf_val(f_inst);
         capt += ", expected from CMIP6_institution_id.json ";
         capt += hdhC::tf_val(str);

         (void) notes->operate(capt) ;
         notes->setCheckStatus(n_CV, pQA->n_fail);
       }
     }
   }

   return ;
}

void
CMIP6_CV::check_experiment_id(std::string id)
{
   disable_requ_glob_att(id);

   Json::Value jsnObj;

   std::string tName(cv_path + "/CMIP6_" + id + ".json") ;
   std::ifstream infile;
   infile.open(tName.c_str());

   infile >> jsnObj;
   infile.close();

   if( jsnObj.isNull() )
       return ; // Missing table? Failed reading?

   // get experiment from global attributes
   std::string g_exp(pGlob->getAttValue(id)) ;

   if( g_exp.size() == 0 )
       return;  // caught and annotated in check_existence()

   Json::Value::Members names = jsnObj[id].getMemberNames() ;
   Json::Value jCV ;
   Json::Value::Members jCV_memb  ;

   for( size_t k=0 ; k < names.size() ; ++k)
   {
      if( names[k] == g_exp )
      {
         jCV = jsnObj[id][names[k]];
         jCV_memb = jCV.getMemberNames();
         break;
      }
   }

   Json::Value obj;
   std::vector<std::string> vs;

   for( size_t k=0 ; k < jCV_memb.size() ; ++k)
   {
       try
       {
         obj = jCV[jCV_memb[k]] ;
       }
       catch (char const*)
       {
         continue ;
       }

       if( obj.empty() )
            continue;

       if( obj.isArray() )
       {
           vs.clear();
           for(Json::ArrayIndex j=0 ; j < obj.size() ; ++j)
              vs.push_back( obj[j].asString() ) ;
       }

       else if(obj.isString())
       {
           vs.clear();
           try
           {
             vs.push_back( obj.asString() );
           }
           catch (char const*)
           {
             continue ;
           }
       }

       else if( obj.isObject() )
       {
           ;  // not for CMIP6
       }

       // Missing global attName was noted before.
       // Thus, a missing one here is not a fault.
       if( hdhC::isAmong(jCV_memb[k], pGlob->attName) )
       {
         std::string f_aVal = pGlob->getAttValue(jCV_memb[k]);

         if( vs.size() && f_aVal.size() )
         {
//            check_experiment_id_value(jCV_memb[k], vs, f_aVal) ;
            if( jCV_memb[k] == "activity_id")
              check_exp_activity_id(jCV_memb[k], vs, f_aVal) ;
/*
            check_exp_additional_allowed_model_components(jCV_memb[k], vs, f_aVal) ;
            check_exp_description(jCV_memb[k], vs, f_aVal) ;
            check_exp_end_year(jCV_memb[k], vs, f_aVal) ;
            check_exp_experiment(jCV_memb[k], vs, f_aVal) ;
            check_exp_min_number_yrs_per_sim(jCV_memb[k], vs, f_aVal) ;
            check_exp_parent_activity_id(jCV_memb[k], vs, f_aVal) ;
            check_exp_parent_experiment_id(jCV_memb[k], vs, f_aVal) ;
            check_exp_required_model_components(jCV_memb[k], vs, f_aVal) ;
            check_exp_start_year(jCV_memb[k], vs, f_aVal) ;
            check_exp_sub_experiment(jCV_memb[k], vs, f_aVal) ;
            check_exp_sub_experiment_id(jCV_memb[k], vs, f_aVal) ;
*/
          }
       }
   }

  return ;
}

void
CMIP6_CV::check_experiment_id_value(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   // file could contain several space-separated entries
   Split x_val(f_aVal);

   // CMIP6-defined entries
   // Json::Value::Members names = jObj_activity_id.getMemberNames() ;
//   Json::Value& jV = jObj_activity_id[id];

   bool is=true;

   for( size_t x=0 ; x < x_val.size() ; ++x )
   {
     // Iterate over the sequence elements.
//     Json::ArrayIndex i;
//     for( i=0; i < jV.size(); ++i )
//        if( jV[i].asString() == x_val[x])
//           break;

     if( hdhC::isAmong(x_val[x], vs) )
     {
        // no period in the filename
        std::string key("2_2");

        if( notes->inq( key, pQA->fileStr) )
        {
          std::string capt("global ");
          capt += hdhC::tf_att(id) ;
          capt += hdhC::tf_val(x_val[x]) ;
          capt += " not defined in CMIP6_experiment_id.json" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus(n_CV, pQA->n_fail);
        }

        add_requ_glob_att(id);
        is=false;
     }
   }

   if(is)
     disable_requ_glob_att(id);

   return;
}

void
CMIP6_CV::check_exp_activity_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   // file could contain several space-separated entries
   Split x_val(f_aVal);

   // CMIP6-defined entries
    Json::Value::Members names = jObj_activity_id.getMemberNames() ;

   for( size_t x=0 ; x < x_val.size() ; ++x )
   {
     // Iterate over the sequence elements.

     if( ! hdhC::isAmong(x_val[x], vs) )
     {
        // no period in the filename
        std::string key("2_2");

        if( notes->inq( key, pQA->fileStr) )
        {
          std::string capt("global ");
          capt += hdhC::tf_att(id, hdhC::colon) ;
          capt += hdhC::tf_val(x_val[x]) ;
          capt += " not defined in CMIP6_experiment_id.json" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus(n_CV, pQA->n_fail);
        }
     }
   }

   return;
}

/*
void
CMIP6_CV::check_exp_additional_allowed_model_components(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_description(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_end_year(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_experiment(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_min_number_yrs_per_sim(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_parent_activity_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_parent_experiment_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_required_model_components(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_start_year(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_sub_experiment(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}

void
CMIP6_CV::check_exp_sub_experiment_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   return;
}
*/

void
CMIP6_CV::check_source_id(std::string id)
{
   disable_requ_glob_att(id);

   Json::Value jsnObj;

   std::string tName(cv_path + "/CMIP6_" + id + ".json") ;
   std::ifstream infile;
   infile.open(tName.c_str());

   infile >> jsnObj;
   infile.close();

   if( ! jsnObj[id].isObject() )
       return ; // Missing table? Failed reading?

   std::string g_exp(pGlob->getAttValue(id)) ;

   if( g_exp.size() == 0 )
       return;  // caught and annotated in check_existence()

   Json::Value::Members names = jsnObj[id].getMemberNames() ;
   Json::Value jCV ;
   Json::Value::Members jCV_memb  ;

   for( size_t k=0 ; k < names.size() ; ++k)
   {
      if( names[k] == g_exp )
      {
         jCV = jsnObj[id][names[k]];
         jCV_memb = jCV.getMemberNames();
         break;
      }
   }

   Json::Value obj;
   std::vector<std::string> vs;

   for( size_t k=0 ; k < jCV_memb.size() ; ++k)
   {
       try
       {
         obj = jCV[jCV_memb[k]] ;
       }
       catch (char const*)
       {
         continue ;
       }

       if( obj.empty() )
            continue;

       if( obj.isArray() )
       {
           vs.clear();
           for(Json::ArrayIndex j=0 ; j < obj.size() ; ++j)
              vs.push_back( obj[j].asString() ) ;
       }

       else if(obj.isString())
       {
           vs.clear();
           try
           {
              vs.push_back( obj.asString() ) ;
           }
           catch (char const*)
           {
             continue ;
           }
       }

       // If a required glob attName is missing, this was noted before. Thus,
       // a missing one here is not a fault.
       if( hdhC::isAmong(jCV_memb[k], pGlob->attName) )
       {
          std::string f_aVal = pGlob->getAttValue(jCV_memb[k]);

          if( vs.size() && f_aVal.size() )
          {
            check_source_id_value(jCV_memb[k], vs, f_aVal);
/*
            check_sid_aerosol(key[k], vs, f_aVal);
            check_sid_atmosphere(key[k], vs, f_aVal);
            check_sid_atmospheric_chemistry(key[k], vs, f_aVal);
            check_sid_cohort(key[k], vs, f_aVal);
            check_sid_institution_id(key[k], vs, f_aVal);
            check_sid_label(key[k], vs, f_aVal);
            check_sid_label_extended(key[k], vs, f_aVal);
            check_sid_land_ice(key[k], vs, f_aVal);
            check_sid_land_surface(key[k], vs, f_aVal);
            check_sid_ocean(key[k], vs, f_aVal);
            check_sid_ocean_biogeochemistry(key[k], vs, f_aVal);
            check_sid_release_year(key[k], vs, f_aVal);
            check_sid_sea_ice(key[k], vs, f_aVal);
            check_sid_source_id(key[k], vs, f_aVal);
*/
          }
       }
  }

  return ;
}

void
CMIP6_CV::check_source_id_value(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
   // file could contain several space-separated entries
   Split x_val(f_aVal);

   bool is=true;

   for( size_t x=0 ; x < x_val.size() ; ++x )
   {
     if( hdhC::isAmong(x_val[x], vs) )
     {
        // no period in the filename
        std::string key("2_2");

        if( notes->inq( key, pQA->fileStr) )
        {
          std::string capt("global ");
          capt += hdhC::tf_att(id, hdhC::colon) ;
          capt += hdhC::tf_val(x_val[x]) ;
          capt += " not defined in CMIP6_source_id.json" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus(n_CV, pQA->n_fail);
        }

        add_requ_glob_att(id);
        is=false;
     }
   }

   if(is)
     disable_requ_glob_att(id);

   return;
}

/*
void
CMIP6_CV::check_sid_aerosol(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_atmosphere(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_atmospheric_chemistry(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_cohort(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_institution_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_label(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_label_extended(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_land_ice(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_land_surface(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_ocean(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_ocean_biogeochemistry(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_release_year(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_sea_ice(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}

void
CMIP6_CV::check_sid_source_id(
    std::string& id, std::vector<std::string>& vs, std::string& f_aVal)
{
    return;
}
*/

void
CMIP6_CV::disable_requ_glob_att(std::string& id)
{
  for( size_t i=0 ; i < vs_requiredGlobalAtts.size() ; ++i )
  {
      if(vs_requiredGlobalAtts[i] == id )
      {
          vs_requiredGlobalAtts[i].clear();
          break;
      }
  }

  return;
}

char
CMIP6_CV::get_CV_tableType(std::string& id)
{
    char c = '\0';

    if( id == "institution_id" )
        c = 's' ;  // the only one

    else if( id == "source_id" )
        c = 'S' ;  // the only one

    else if( id == "experiment_id" )
        c = 'E' ;  // the only one
    else
        c = 'a';

    return c;
}

Json::Value
CMIP6_CV::getJsonValue(std::string& id)
{
   Json::Value jv;

   std::string str("/CMIP6_");
   // a bug or a feature???
   if( id == "mip_era" )
       str = "/" ;

   std::string tName(cv_path + str + id + ".json") ;
   std::ifstream infile;
   infile.open(tName.c_str());

   infile >> jv;
   infile.close();

   return jv;  // NULL for error
}

void
CMIP6_CV::readJsonFiles(void)
{
    Json::Value jv;

    std::vector<std::string> ids;
    ids.push_back("activity_id");
    ids.push_back("frequency");
    ids.push_back("grid_label");
    ids.push_back("institution_id");
    ids.push_back("nominal_resolution");
    ids.push_back("realm");
    ids.push_back("required_global_attributes");
    ids.push_back("source_type");
    ids.push_back("table_id");
    ids.push_back("mip_era");

    size_t i=0;
    jObj_activity_id = getJsonValue(ids[i++]);
    jObj_frequency = getJsonValue(ids[i++]);
    jObj_grid_label = getJsonValue(ids[i++]);
    jObj_institution_id = getJsonValue(ids[i++]);
    jObj_nominal_resolution = getJsonValue(ids[i++]);
    jObj_realm = getJsonValue(ids[i++]);
    jObj_required_global_attributes = getJsonValue(ids[i++]) ;
    jObj_source_type = getJsonValue(ids[i++]);
    jObj_table_id = getJsonValue(ids[i++]);
    jObj_mip_era = getJsonValue(ids[i++]);

    return;
}

void
CMIP6_CV::run(void)
{
  // read all json tables, but CMIP6_source_id.json and CMIP6_experiment.json
  readJsonFiles();

  //get names of required global attributes
  std::string str("required_global_attributes") ; // is used by reference
  (void) check_array(str);

  // check required global attributes
  check_existence();

  // the corresponding json files include more attributes
  check_experiment_id("experiment_id");
  check_source_id("source_id");

  // some reqGlobAtts could habe been cleared in the meanwhile
  size_t i;
  for( i=0 ; i < vs_requiredGlobalAtts.size() ; ++i )
  {
      if( vs_requiredGlobalAtts[i] == "institution" )
          continue;  // together with institution_id

      char cv_type = get_CV_tableType(vs_requiredGlobalAtts[i]);

      if( cv_type == 'a' && check_array(vs_requiredGlobalAtts[i]) )
        ;
      else if( cv_type == 's' )
        check_string(vs_requiredGlobalAtts[i]);

      else
      {
        // no json table; plain comparison
        // meta from file
        std::string& id = vs_requiredGlobalAtts[i] ;

        std::string f_val = pGlob->getAttValue(id);
          ; //ToDo specific checks
      }

  }

}

// --------- class

DRS_CV::DRS_CV(QA* p)
{
  pQA = p;

  notes = pQA->notes;

  enabledCompletenessCheck=true;

  applyOptions(pQA->optStr);
}

void
DRS_CV::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "dIP"
          || split[0] == "dataInProduction"
              || split[0] == "data_in_production" )
     {
        // effects completeness test in testPeriod()
        enabledCompletenessCheck=false;
        continue;
     }
   }

   return;
}

void
DRS_CV::checkFilename(std::string& fName, struct DRS_CV_Table& drs_cv_table)
{
  Split x_filename;
  x_filename.setSeparator("_");
  x_filename.enableEmptyItems();
  x_filename = fName ;

  checkVariableName(x_filename[0]) ;

//  checkFilenameGeographic(x_filename);

  std::string frq = pQA->qaExp.getFrequency();

  if( testPeriod(x_filename) && frq.size() && frq != "fx" )
  {
    // no period in the filename
    std::string key("1_6a");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("filename requires a period") ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
    }
  }

  checkFilenameEncoding(x_filename, drs_cv_table);

  return;
}

void
DRS_CV::checkFilenameEncoding(Split& x_filename, struct DRS_CV_Table& drs_cv_table)
{
  // fNameEncodingStr: name of the encoding type
  // fNameEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  if( x_filename.size() == 0 )
    return;

  // components of the current path, these are given in the reverse order.
  Split& drs = x_filename;

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";
  std::string t;

  size_t enc_sz = drs_cv_table.fNameEncoding.size() ;

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  std::vector<std::vector<size_t> > specialFaultIx ;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;
    specialFaultIx.push_back( std::vector<size_t>() );

    //x_e.setSeparator("_");
    x_e.setSeparator("/");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    // could have a trailing ".nc" item; if yes, then remove this beforehand
    if( drs_cv_table.fNameEncoding[ds].rfind(".nc") < std::string::npos )
    {
      std::string& t = drs_cv_table.fNameEncoding[ds];
      // is it trailing?
      if( t.substr(t.size()-3) == ".nc" )
        x_e = t.substr(0, t.size()-3) ;
    }
    else
      x_e = drs_cv_table.fNameEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->drs_cv_table.table_DRS_CV.getFile() ;
        capt += ": encoding ";
        capt += hdhC::tf_assign("item", x_e[x]) + " not found in CV";
        if( notes->inq( key, drsF) )
        {
          (void) notes->operate(capt) ;
          notes->setCheckStatus("DRS", pQA->n_fail);
        }
      }

      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "member_id" )
          gM[x_e[x]] = getEnsembleMember() ;

        else if( x_e[x] == "gridspec" )
          gM[x_e[x]] = "gridspec" ;

        else if( x_e[x] == "temporal subset" )
          gM[x_e[x]] = n_ast ;

        else if( x_e[x] == "geographical info" )
          gM[x_e[x]] = n_ast ;

        else
          gM[x_e[x]] = globalVar.getAttValue(x_e[x]) ;
      }
      else
        gM[x_e[x]] = globalVar.getAttValue(cvMap[x_e[x]]) ;
    }

    // special for gridspec filenames
    if( drs_cv_table.fNameEncodingStr[ds] == "GRIDSPEC" )
    {
      for( size_t i=0 ; i < drs.size() ; ++i)
      {
        // ensemble member
        if( x_e[i] == "member_id" )
        {
          t = gM[ x_e[i] ];
          if( t != "r0i0p0" )
            specialFaultIx[ds].push_back(i);
        }
      }
    }


    // append * twice in case of no geo-info nor period;
    // surplusses are safe
    drs.append("*");
    drs.append("*");

    //count coincidences between path components and the DRS
    for( size_t jx=0 ; jx < x_e.size() ; ++jx)
    {
      t = gM[ x_e[jx] ];

      if( jx == drs.size() )
        break;

      if( drs[jx] == t || t == n_ast )
        ++countCI[ds];
    }

    if( ! specialFaultIx[ds].size() )
      if( countCI[ds] == x_e.size() )
        return;  // check passed; take into account two optional items
  }

  // find the encoding with maximum number of coincidences, which
  // is assumed to correspond to the current encoding type.
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  // find details of the faults
  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::vector<std::string> text;
  std::vector<std::string> keys;

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    t = gM[ x_e[i] ];

    if( hdhC::isAmong(i, specialFaultIx[m]) )
    {
      if( x_e[i] == "frequency" )
      {
        keys.push_back("1_5b");
        text.push_back( "A gridspec file must have frequency fx" );
      }

      if( x_e[i] == "member_id" )
      {
        keys.push_back("1_5a");
        text.push_back( "A gridspec file must have ensemble member r0i0p0" );
      }
    }
  }

  std::string txt;
  findFN_faults(drs, x_e, gM, txt) ;
  if( txt.size() )
  {
     text.push_back(txt);
     keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS CV filename:");
    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], "DRS") )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckStatus(drsF, pQA->n_fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkFilenameGeographic(Split& x_filename)
{
  // the geographical indicator should be the last item
  int i;
  int last = x_filename.size()-1 ;

  for( i=last ; i >= 0 ; --i )
  {
    if( x_filename[i].size() > 1 )
    {
      if( x_filename[i][0] == 'g' && x_filename[i][1] == '-' )
        break;
    }
  }

  if( i == -1 )
    return ;  // no geo-indicator

  Split x_geo(x_filename[i], "-");

  std::vector<std::string> key;
  std::vector<std::string> capt;
  std::string cName("geographical indicator");
  if( i != last )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7a");
    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " should appear last in the filename" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
    }
  }

  if( x_geo.size() < 2 )
  {
    // the geographical indicator should be the last item
    std::string bkey("1_7b");

    if( notes->inq( key[i], pQA->fileStr))
    {
      std::string capt(cName);
      capt += " g-XXXX[-YYYY]: syntax fault" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
    }

    return ;
  }

  // valid stand-alone '-' separated words
  std::vector<std::string> kw;
  kw.push_back(CMOR::n_global);
  kw.push_back("lnd");
  kw.push_back("ocn");
  kw.push_back("zonalavg");
  kw.push_back("areaavg");

  size_t kw_pos_zzz=3;

  // entries (1st char): (b)ounding-box, (r)egion, (t)ype, (f)ault
  std::vector<char> seq ;

  for( size_t ix=1 ; ix < x_geo.size() ; ++ix )
  {
    if( ix == 1 )
    {
      // a) bounding box?
      Split x_box;
      x_box.setSeparator("lat");
      x_box.addSeparator("lon");
      x_box.setItemsWithSeparator();
      x_box = x_geo[ix];

      for(size_t n=0 ; n < x_box.size() ; ++n )
      {
        bool isLat=false ;
        bool isLon=false ;
        double val=0.;

        if( x_box[n] == "lat" )
        {
          isLat = true ;
          isLon = false ;
          val=90.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }
        else if( x_box[n] == "lon" )
        {
          isLon = true ;
          isLat = false ;
          val=180.;
          if( seq.size() == 0 )
            seq.push_back('b');
          continue;
        }

        Split x_term;
        x_term.setSeparator(":alnum:");
        x_term = x_box[n];

        for( size_t jx=0; jx < x_term.size() ; ++jx )
        {
          // a number is expected
          if( hdhC::isDigit(x_term[jx]) )
          {
            if( x_term[jx].find('.') < std::string::npos )
            {
              key.push_back("1_7c");
              capt.push_back(cName + ": numbers should be rounded to the nearest integer");
            }

            if( hdhC::string2Double(x_term[jx]) > val )
            {
              key.push_back("1_7d");
              capt.push_back(cName);
              if(isLat)
                capt.back() += ": latitude value should not exceed 90 degr";
              else if(isLon)
                capt.back() += ": longitude value should not exceed 180 degr";
            }
          }

          // range indicator
          ++jx;
          bool is_SNWE_fault=false;

          if( isLat && ! (x_term[jx] == "S" || x_term[jx] == "N") )
            is_SNWE_fault=true;
          else if( isLon && ! (x_term[jx] == "E" || x_term[jx] == "W") )
            is_SNWE_fault=true;

          if( is_SNWE_fault )
          {
            key.push_back("1_7e");
            capt.push_back(cName + ": invalid bounding-box ");
          }
        }
      }
    }  // ix==1

    if( ix ==1 && seq.size() )
      continue;

    size_t k;
    for(k=0 ; k < kw.size() ; ++k )
      if( kw[k] == x_geo[ix] )
        break;

    if( k < kw_pos_zzz )
      seq.push_back('r') ;
    else if( k < kw.size() )
      seq.push_back('t') ;
    else
      seq.push_back('f') ;

  } // geo_ix

  // a type requires a preceding BB or region
  for( size_t i=0 ; i < seq.size() ; ++i )
  {
    if( seq[i] == 't')
    {
      bool isA = !( i && (seq[i-1] == 'b' || seq[i-1] == 'r') ) ;
      bool isB = !( i && (seq[i-1] == 'f'  ) ) ;
      if( isA && isB )
      {
        key.push_back("1_7g");
        capt.push_back(cName + ": invalid specifier ");
        capt.back() += "g-XXXX[-yyy][-zzz]: given zzz, but missing XXXX";
      }
    }
  }

  if( hdhC::isAmong('f', seq) )
  {
    // 1st index for 'g'
    for( size_t i=1 ; i < x_geo.size() ; ++i )
    {
      size_t j;
      for( j=0 ; j < kw.size() ; ++j )
      {
        if( kw[j] == x_geo[i] )
          break;

        std::string t(x_geo[i].substr(0,3));
        if( t == "lat" || t == "lon" )
          break;
      }

      if( j == kw.size() )
      {
        key.push_back("1_7f");
        capt.push_back(cName + ": invalid specifier");
        capt.back() += hdhC::tf_val(x_geo[i]) ;
        capt.back() += ", valid ";
        if( kw.size() > 1 )
          capt.back() += "are";
        else
          capt.back() += "is";
        capt.back() += hdhC::tf_val( hdhC::getVector2Str(kw) );

      }
    }
  }


  for( size_t ii=0 ; ii < key.size() ; ++ii )
  {
    if( notes->inq( key[ii], pQA->fileStr))
    {
      (void) notes->operate(capt[ii]) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
    }
  }

  return ;
}

void
DRS_CV::checkMIPT_tableName(Split& x_filename)
{
  // Note: filename:= name_CMOR-MIP-table_... .nc
  if( x_filename.size() < 2 )
    return ;  //corrupt filename

  // table sheet name from global attributes has been checked

  // compare file entry to the one in global header attribute table_id
  if( x_filename[1] != QA::tableId )
  {
    std::string key("7_8");

    if( notes->inq( key, "MIP") )
    {
      std::string capt("Ambiguous MIP table names, found ") ;
      capt += hdhC::tf_assign("MIP-table(file)", x_filename[1]) ;
      capt += " vs. global ";
      capt += hdhC::tf_att(CMOR::n_table_id, QA::tableId);

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    // try the filename's MIP table name
    if( QA::tableId.size() == 0 )
      QA::tableId = pQA->qaExp.getMIP_tableName(x_filename[1]) ;
  }

  return ;
}

void
DRS_CV::checkNetCDF(NcAPI* p_nc)
{
  // NC_FORMAT_CLASSIC (1)
  // NC_FORMAT_64BIT   (2)
  // NC_FORMAT_NETCDF4 (3)
  // NC_FORMAT_NETCDF4_CLASSIC  (4)

  if(!p_nc)
      p_nc=&(pQA->pIn->nc);

  NcAPI& nc = *p_nc;

  int fm = nc.inqNetcdfFormat();

  if( fm == 1 || fm == 4 )
    return;

  std::string s("");

  bool is=false;

  if( fm == 1 )
  {
    is=true;
    s = "3, NC_FORMAT_CLASSIC";
  }
  else if( fm == 2 )
  {
    is=true;
    s = "3, NC_FORMAT_64BIT";
  }
  else if( fm == 3 )
  {
    is=true;
    s = "4, ";
  }
  else if( fm == 4 )
    s = "4, classic";

  if( fm > 2 )
  {
    if( ! nc.inqDeflate())
    {
      s+= " deflated (compressed)";
      is=true;
    }
  }

  if(is)
  {
    std::string key("12");
    if( notes->inq( key, pQA->fileStr ) )
    {
      std::string capt("format does not conform to netCDF classic, found") ;
      capt += s;

      (void) notes->operate( capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
    }
  }

  return;
}

void
DRS_CV::checkPath(std::string& path, struct DRS_CV_Table& drs_cv_table)
{
  // pathEncodingStr: name of the encoding type
  // pathEncoding:     sequence of DRS path components
  // encodingMap:      name in encoding vs. name of global attribute (or *)

  size_t enc_sz = drs_cv_table.pathEncoding.size() ;

  if( path.size() == 0 || enc_sz == 0 )
    return;

  // components of the current path, these are given in the reverse order.
//  path= "/hdh/data/CMIP5/CMIP5/output/MPI-M/MPI-ESM-LR/hysterical/day/artmos/r1i1p1/tas/";
  Split drs(path, "/");

  Variable& globalVar = pQA->pIn->variable[ pQA->pIn->varSz ] ;
  std::string n_ast="*";

  Split x_enc[enc_sz];
  std::vector<size_t> countCI(enc_sz, 0);
  std::map<std::string, std::string> globMap[enc_sz] ;
  std::vector<std::string> text;
  std::vector<std::string> keys;
  size_t drsBeg;

  for( size_t ds=0 ; ds < enc_sz ; ++ds)
  {
    Split& x_e = x_enc[ds] ;
    std::map<std::string, std::string>& gM = globMap[ds] ;

    x_e.setSeparator("/");

    std::map<std::string, std::string>& cvMap = drs_cv_table.cvMap ;

    x_e = drs_cv_table.pathEncoding[ds] ;

    for(size_t x=0 ; x < x_e.size() ; ++x )
    {
      if( cvMap.count(x_e[x]) == 0 )
      {
        std::string key("7_3");
        std::string capt("Fault in table ");
        capt += pQA->drs_cv_table.table_DRS_CV.getFile() ;
        capt += ": encoding not available in CV, found " ;
        capt += hdhC::tf_assign("item", x_e[x]) ;

        if( notes->inq( key, drsP) )
        {
          (void) notes->operate(capt) ;
          notes->setCheckStatus(drsP, pQA->n_fail);
        }
      }

      std::string s_tmp;
      if( cvMap[x_e[x]] == n_ast )
      {
        if( x_e[x] == "activity_id" )
        {
          s_tmp = globalVar.getAttValue(x_e[x]) ;
          // only the first element of vector
          if( s_tmp.find(" ") )
          {
              Split xs_tmp(s_tmp);
              s_tmp = xs_tmp[0];
          }
        }
        else if( x_e[x] == "member_id" )
          s_tmp = getEnsembleMember() ;
        else if( x_e[x] == "gridspec" )
          s_tmp = "gridspec" ;
        else
          s_tmp = globalVar.getAttValue(x_e[x]) ;
      }
      else
        s_tmp = globalVar.getAttValue(cvMap[x_e[x]]) ;

      if( hdhC::Lower()(s_tmp) == "none" )
          s_tmp.clear();
      gM[x_e[x]] = s_tmp ;
    }

    std::string t;
    int ix;
    if( (ix = getPathBegIndex( drs, x_e, gM )) == -1 )
      continue;

    drsBeg = ix;
    size_t i;

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      t = gM[ x_e[j] ];

      if( (i = drsBeg+j) < drs.size() )
      {
        if( drs[i] == t || t == n_ast )
          ++countCI[ds];
        else if( x_e[j] == "version")
        {
          if( drs[i][0] == 'v'
                 && drs[i].size() == 9
                    && hdhC::isDigit(drs[i].substr(1))  )
            ++countCI[ds];
          else if(x_e[j] == "mip_era")
          {
            if( notes->inq( "1_1a", pQA->fileStr, "INQ_ONLY") )
            {
              if( t == hdhC::Upper()(drs[i]) )
                ++countCI[ds];
            }
          }
        }
      }
      else
        drs.append(n_ast); // who knows
    }

    if( countCI[ds] == x_e.size() )
      return;  // check passed
  }

  // find the encoding with maximum number of coincidences
  size_t m=0;
  size_t mx=countCI[0];
  for( size_t ds=1 ; ds < countCI.size() ; ++ds )
  {
    if( countCI[ds] > mx )
    {
      mx = countCI[ds] ;
      m=ds;
    }
  }

  Split& x_e = x_enc[m] ;
  std::map<std::string, std::string>& gM = globMap[m] ;

  std::string txt;
  findPath_faults(drs, static_cast<int>(drsBeg),  x_e, gM, txt) ;
  if( txt.size() )
  {
    text.push_back(txt);
    keys.push_back("1_2");
  }

  if( text.size() )
  {
    std::string capt("DRS path:");

    for(size_t i=0 ; i < text.size() ; ++i )
    {
      if( notes->inq( keys[i], pQA->fileStr) )
      {
        (void) notes->operate(capt+text[i]) ;
        notes->setCheckStatus(drsP, pQA->n_fail);
      }
    }
  }

  return;
}

void
DRS_CV::checkVariableName(std::string& f_vName)
{
  if( f_vName.find('-') < std::string::npos )
  {
    std::string key("1_4c");

    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt(hdhC::tf_var(f_vName));
      capt += "in the filename should not contain a hyphen" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return;
}

void
DRS_CV::findFN_faults(Split& drs, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();
  int drsSz = drs.size() ;

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    if( drsSz == x_eSz )
    {
      text = " check failed, suspicion of a missing item in the filename, found" ;
      text += hdhC::tf_val(drs.getStr()) ;

      return;
    }

    if( drs[j] == CMOR::n_output )
    if( !(drs[j] == t || t == n_ast) )
    {
      text = " check failed, found " ;
      text += hdhC::tf_assign(x_e[j],drs[j]) ;
      text += ", expected" ;
      text += hdhC::tf_val(t) ;

      return;
    }
  }

  return ;
}

void
DRS_CV::findPath_faults(Split& drs, int drsBeg, Split& x_e,
                   std::map<std::string,std::string>& gM,
                   std::string& text)
{
  std::string t;
  std::string n_ast="*";
  int x_eSz = x_e.size();

  if( drsBeg < 0)
  {
    text = " check failed" ;
    return; // fewer path items than expected
  }

  for( int j=0 ; j < x_eSz ; ++j)
  {
    t = gM[ x_e[j] ];

    int i = drsBeg+j ;

    if( i == -1 )
    {
      text = " suspicion of a missing item in the path, found" ;
      text += hdhC::tf_val(drs.getStr()) ;
      break;
    }

    if( !( drs[i] == t || t == n_ast) )
    {
      text = " path component " ;
      text += hdhC::tf_assign(x_e[j],drs[drsBeg+j]) ;
      text += " does not match global attribute value" ;
      text += hdhC::tf_val(t) ;
      break;
    }
  }

  return ;
}

std::string
DRS_CV::getEnsembleMember(void)
{
  if( ensembleMember.size() )
    return ensembleMember;

  // ensemble member
  int ix;
  Variable& glob = pQA->pIn->variable[ pQA->pIn->varSz ] ;

  std::string sub_exp(glob.getAttValue("sub_experiment"));
  std::string variant(glob.getAttValue("variant_label"));

  if( sub_exp.size() && hdhC::Lower()(sub_exp) != "none")
    ensembleMember = sub_exp + "-" ;
  ensembleMember += variant;

  if( !ensembleMember.size() )
  {
    bool is=false;

    ensembleMember = "r" ;
    if( (ix = glob.getAttIndex("realization_index")) > -1 )
      ensembleMember += glob.attValue[ix][0] ;
    else
      is=true;

    ensembleMember += 'i' ;
    if( (ix = glob.getAttIndex("initialization_index")) > -1 )
      ensembleMember += glob.attValue[ix][0] ;
    else
      is=true;

    ensembleMember += 'p' ;
    if( (ix = glob.getAttIndex("physics_index")) > -1 )
      ensembleMember += glob.attValue[ix][0] ;
    else
      is=true;

    ensembleMember += 'f' ;
    if( (ix = glob.getAttIndex("forcing_index")) > -1 )
      ensembleMember += glob.attValue[ix][0] ;
    else
      is=true;

    if(is)
        ensembleMember.clear();
  }

  return ensembleMember ;
}

int
DRS_CV::getPathBegIndex(
    Split& drs, Split& x_e,
    std::map<std::string, std::string>& gM )
{
  int ix=-1;
  bool isActivity; // applied case-insensivity

  for( size_t i=0 ; i < drs.size() ; ++i)
  {
    std::string s(drs[i]);

    for( size_t j=0 ; j < x_e.size() ; ++j)
    {
      if( x_e[j] == CMOR::n_activity )
      {
        isActivity=true;
        s = hdhC::Upper()(s);
      }
      else
        isActivity=false;

      if( s == gM[ x_e[j] ]  )
      {
        ix = static_cast<int>(i);

        // the match could be a leading false one, requested is the last one
        for( ++i ; i < drs.size() ; ++i)
        {
          s = drs[i];

          if(isActivity)
            s = hdhC::Upper()(s);

          if( s == gM[ x_e[j] ]  )
            ix = static_cast<int>(i);
        }

        return ix;
      }
    }
  }

  return ix;
}

bool
DRS_CV::isInstantTime(void)
{

  if( pQA->qaExp.getFrequency() == "6hr" )
    return true;

  bool is=true;
  for( size_t i=0 ; i < pQA->qaExp.varMeDa.size() ; ++i )
    if( pQA->qaExp.varMeDa[i].attMap[CMOR::n_cell_methods].size()
          && pQA->qaExp.varMeDa[i].attMap[CMOR::n_cell_methods] != "time: point" )
      is=false;

  return is;
}

void
DRS_CV::run(void)
{
  DRS_CV_Table& drs_cv_table = pQA->drs_cv_table ;

  // check the path
  if( pQA->isCheckDRS_P )
  {
     drsP = "DRS(P)" ;
     checkPath(pQA->pIn->file.path, drs_cv_table) ;
  }

  // compare filename to netCDF global attributes
  if( pQA->isCheckDRS_F )
  {
     drsF = "DRS(F)" ;
     checkFilename(pQA->pIn->file.basename, drs_cv_table);
  }

  // is it NetCDF-3 classic?
  //checkNetCDF();

  return;
}

bool
DRS_CV::testPeriod(Split& x_f)
{
  // return true, if a file is supposed to be not complete.
  // return false, a) if there is no period in the filename
  //               b) if an error was found
  //               c) times of period and file match

  // The time value given in the first/last record is assumed to be
  // in the range of the period of the file, if there is any.

  // If the first/last date in the filename period and the
  // first/last time value match within the uncertainty of the
  // time-step, then the file is complete.
  // If the end of the period exceeds the time data figure,
  // then the nc-file is considered to be not completely processed.

  // Does the filename have a trailing date range?
  std::vector<std::string> sd;
  sd.push_back( "" );
  sd.push_back( "" );

  // is it formatted as expected?
  if( testPeriodFormat(x_f, sd) )
    return true;

  // now that we have found two candidates for a date
  // compose ISO-8601 strings
  std::vector<Date> period;
  pQA->qaTime.getDRSformattedDateRange(period, sd);

  // necessary for validity (not sufficient)
  if( period[0] > period[1] )
  {
     std::string key("1_6c");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("invalid period in the filename, found ");
       capt += hdhC::tf_val(sd[0] + "-" + sd[1]);

       (void) notes->operate(capt) ;
       notes->setCheckStatus(drsF, pQA->n_fail );
     }

     return false;
  }

  Date* pDates[6];
  pDates[0] = &period[0];  // StartTime in the filename
  pDates[1] = &period[1];  // EndTime in the filename

  // index 2: date of first time value
  // index 3: date of last  time value
  // index 4: date of first time-bound value, if available; else 0
  // index 5: date of last  time-bound value, if available; else 0

  for( size_t i=2 ; i < 6 ; ++i )
    pDates[i] = 0 ;

  if( pQA->qaTime.isTimeBounds)
  {
    pDates[4] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.firstTimeBoundsValue[0] != 0 )
      pDates[4]->addTime(pQA->qaTime.firstTimeBoundsValue[0]);

    pDates[5] = new Date(pQA->qaTime.refDate);
    if( pQA->qaTime.lastTimeBoundsValue[1] != 0 )
      pDates[5]->addTime(pQA->qaTime.lastTimeBoundsValue[1]);
  }
  else
  {
    if( pQA->qaTime.time_ix > -1 &&
        ! pQA->pIn->variable[pQA->qaTime.time_ix].isInstant )
    {
      if( !isInstantTime() )
      {
        std::string key("3_17");
        if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
        {
          std::string capt(hdhC::tf_var("time_bnds"));
          capt += "is missing" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus(drsF, pQA->n_fail);
        }
      }
    }
  }

  pDates[2] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.firstTimeValue != 0. )
    pDates[2]->addTime(pQA->qaTime.firstTimeValue);

  pDates[3] = new Date(pQA->qaTime.refDate);
  if( pQA->qaTime.lastTimeValue != 0. )
    pDates[3]->addTime(pQA->qaTime.lastTimeValue);

  // alignment of of contained dates and those in the filename
  // the booleanx indicate faults
//  bool is_t_beg = is_t_end = is_tb_beg = is_tb_end = false;
  bool isFault[4];
  for(size_t i=0 ; i < 4 ; ++i )
    isFault[i]=false;

  // time value: left-side
  Date myDate( *pDates[2] );
  myDate.addTime(-pQA->qaTime.refTimeStep);
  isFault[0] = myDate == *pDates[0] ;

  // time value: right-side
  myDate = *pDates[3] ;
  myDate.addTime(pQA->qaTime.refTimeStep);
  isFault[1] = myDate ==*pDates[1] ;

  if(pQA->qaTime.isTimeBounds)
  {
    // time_bounds: left-side
    myDate = *pDates[4] ;
    myDate.addTime(pQA->qaTime.refTimeStep);
    isFault[2] = myDate == *pDates[0] ;

    // time_bounds: right-side
    myDate = *pDates[5] ;
    myDate.addTime(-pQA->qaTime.refTimeStep);
    isFault[3] = myDate == *pDates[1] ;
  }

  // the annotations
  if( testPeriodAlignment(sd, pDates, isFault) )
  {
    std::string key("1_6g");

    if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
    {
      std::string capt("Warn: Filename's period: No time_bounds, ");
      capt += "StartTime-EndTime rounded to time values";

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail );
    }
  }
  else if( testPeriodDatesFormat(sd) ) // format of period dates.
  {
    // period requires a cut specific to the various frequencies.
    std::vector<std::string> text ;
    testPeriodPrecision(sd, text) ;

    if( text.size() )
    {
      std::string key("1_6d");
      std::string capt("period in the filename with wrong precision") ;

      for( size_t i=0 ; i < text.size() ; ++i )
      {
        if( notes->inq( key, pQA->qaExp.getVarnameFromFilename()) )
        {
          (void) notes->operate(capt + text[i]) ;
          notes->setCheckStatus(drsF, pQA->n_fail );
        }
      }
    }
  }

  // note that indices 0 and 1 belong to a vector
  for(size_t i=2 ; i < 6 ; ++i )
    if( pDates[i] )
      delete pDates[i];

  // complete
  return false;
}

bool
DRS_CV::testPeriodAlignment(std::vector<std::string>& sd, Date** pDates, bool b[])
{
  // some pecularities of CMOR, which will probably not be modified
  // for a behaviour as expected.
  // The CORDEX Archive Design doc states in Appendix C:
  // "..., for using CMOR, the StartTime-EndTime element [in the filename]
  //  is based on the first and last time value included in the file ..."

  // if a test for a CMOR setting fails, testing goes on
  if( *pDates[0] == *pDates[2] && *pDates[1] == *pDates[3] )
      return true;

  for(size_t i=0 ; i < 2 ; ++i)
  {
    // i == 0: left; 1: right

    // skip for the mode of checking during production
    if( i && !pQA->isFileComplete )
       continue;

    if( !( !b[0+i] || !b[2+i] ) )
    {
      std::string key("1_6f");
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string token;

        std::string capt("Misaligned ");
        if( i == 0 )
          capt += "begin" ;
        else
          capt += "end" ;
        capt += " of periods in filename and ";

        size_t ix;

        if( pQA->qaTime.isTimeBounds )
        {
          capt="time bounds: ";
          ix = 4 + i ;
        }
        else
        {
          capt="time values: ";
          ix = 2 + i ;
        }

        capt += sd[i] ;
        capt += " vs. " ;
        capt += pDates[ix]->str();

        (void) notes->operate(capt) ;
        notes->setCheckStatus(drsF, pQA->n_fail );
      }
    }
  }

  return false;
}

void
DRS_CV::testPeriodPrecision(std::vector<std::string>& sd,
                  std::vector<std::string>& text)
{
  // Partitioning of files check are equivalent.
  // Note that the format was tested before.
  // Note that desplaced start/end points, e.g. '02' for monthly data, would
  // lead to a wrong cut.

  if( sd[0].size() != sd[1].size() )
  {
    text.push_back(" 1st and 2nd date are different") ;
    return;
  }

/*
  // precision corresponds to the MIP table
  std::vector<size_t> ix;

  // a) yyyy
  if( QA::tableId == QA_Exp::MIP_tableNames[1] )
  {
    if( sd[0].size() != 4 )
      text.push_back(", expected yyyy, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // b) ...mon, aero, Oclim, and cfOff
  ix.push_back(2);
  ix.push_back(3);
  ix.push_back(4);
  ix.push_back(5);
  ix.push_back(6);
  ix.push_back(7);
  ix.push_back(8);
  ix.push_back(13);
  ix.push_back(17);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( QA::tableId == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 6 )
        text.push_back(", expected yyyyMM, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // c) day, cfDay
  if( QA::tableId == QA_Exp::MIP_tableNames[9]
        || QA::tableId == QA_Exp::MIP_tableNames[14] )
  {
    if( sd[0].size() != 8 )
      text.push_back(", expected yyyyMMdd, found " + sd[0] + "-" + sd[1]) ;

    return;
  }

  // d) 6hr..., 3hr, and cf3hr
  ix.clear();
  ix.push_back(10);
  ix.push_back(11);
  ix.push_back(12);
  ix.push_back(15);

  for(size_t i=0 ; i < ix.size() ; ++i )
  {
    if( QA::tableId == QA_Exp::MIP_tableNames[ix[i]] )
    {
      if( sd[0].size() != 12 )
        text.push_back(", expected yyyyMMddhhmm, found " + sd[0] + "-" + sd[1]) ;

      return;
    }
  }

  // e) cfSites
  if( QA::tableId == QA_Exp::MIP_tableNames[16] )
  {
    if( sd[0].size() != 14 )
      text.push_back(", expected yyyyMMddhhmmss, found " + sd[0] + "-" + sd[1]) ;
  }
*/

  return;
}

bool
DRS_CV::testPeriodDatesFormat(std::vector<std::string>& sd)
{
  // return: true means go on for testing the period cut
  // partitioning of files
  if( sd.size() != 2 )
    return false;  // no period; is variable time invariant?

  std::string frequency(pQA->qaExp.getFrequency());
  std::string str;

  if( frequency == "3hr" || frequency == "6hr" )
  {
      if( sd[0].size() != 10 || sd[1].size() != 10 )
      {
        str += "YYYYMMDDhh for ";
        if( frequency == "3hr" )
          str += "3";
        else
          str += "6";
        str += "-hourly time step";
      }
  }
  else if( frequency == "day" )
  {
      if( sd[0].size() != 8 || sd[1].size() != 8 )
        str += "YYYYMMDD for daily time step";
  }
  else if( frequency == "mon" || frequency == "sem" )
  {
     if( sd[0].size() != 6 || sd[1].size() != 6 )
     {
        str += "YYYYMM for ";
        if( frequency == "mon" )
          str += "monthly";
        else
          str += "seasonal";
        str += " data";
     }
  }

  if( str.size() )
  {
    std::string key("1_6e");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("period in filename of incorrect format, found ");
        capt += sd[0] + "-" +  sd[1];
        capt += " expected " + str ;

        (void) notes->operate(capt) ;
        notes->setCheckStatus(drsF, pQA->n_fail );
     }
  }

  return true;
}

bool
DRS_CV::testPeriodFormat(Split& x_f, std::vector<std::string>& sd)
{
  int x_fSz=x_f.size();

  if( ! x_fSz )
    return true; // trapped before

  // any geographic subset? Even with wrong separator '_'?
  if( x_f[x_fSz-1].substr(0,2) == "g-" )
    --x_fSz;
  else if( x_f[x_fSz-1] == "g" )
    --x_fSz;
  else if( x_fSz > 1 && x_f[x_fSz-2].substr(0,2) == "g" )
    x_fSz -= 2 ;

  if( x_fSz < 1 )
    return true;

  if( x_f[x_fSz-1] == "clim" || x_f[x_fSz-1] == "ave" )
  {
    // Wrong separator for appendix clim or ave, found '_'
    std::string key = "1_6b" ;
    if( notes->inq( key, pQA->fileStr) )
    {
      std::string capt("Wrong separation of filename's period's appendix");
      capt += hdhC::tf_val(x_f[x_fSz-1]);
      capt += ", found underscore";

      (void) notes->operate(capt) ;
      notes->setCheckStatus(drsF, pQA->n_fail );
    }

    --x_fSz;
  }

  if( x_fSz < 1 )
    return true;

  Split x_last(x_f[x_fSz-1],'-') ;
  int x_lastSz = x_last.size();

  // minimum size of x_last could never be zero

  // elimination of 'ave' or 'clim' separated by a dash
  if( x_last[x_lastSz-1] == "clim" || x_last[x_lastSz-1] == "ave" )
    --x_lastSz ;

  bool isRegular=true;
  if( x_lastSz == 2 )
  {
    // the regular case
    std::vector<int> ix;
    if( hdhC::isDigit(x_last[0]) )
      sd[0]=x_last[0] ;
    else
    {
      ix.push_back(0);
      isRegular=false;
    }

    if( hdhC::isDigit(x_last[1]) )
      sd[1]=x_last[1] ;
    else
    {
      isRegular=false;
      ix.push_back(1);
    }

    if( ix.size() == 2 )
      return true;  // apparently not a period
    else if( ix.size() == 1 )
    {
      // try for clim or ave without any separator with a wrong one
      size_t pos;
      std::string f(x_last[ix[0]]);
      if( (pos=f.rfind("clim")) < std::string::npos
             || (pos=f.rfind("ave")) < std::string::npos )
      {
        if( pos && hdhC::isDigit( f.substr(0, pos-1) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos-1) ;
        }
        else if( pos && hdhC::isDigit( f.substr(0,pos) ) )
        {
          if( ix[0] == 1 )
            isRegular = true;

          sd[ix[0]] = f.substr(0,pos) ;
        }
        else
          isRegular=false;

        if(ix.size())
        {
          // Wrong separator for appendix clim or ave, found '_'
          std::string key("1_6b") ;
          if( notes->inq( key, pQA->fileStr) )
          {
            std::string capt("Wrong separation of filename's period's appendix");
            capt += hdhC::tf_val(x_last[ix[0]]);

            (void) notes->operate(capt) ;
            notes->setCheckStatus(drsF, pQA->n_fail );
          }
        }
      }
    }
  }
  else
    isRegular=false;

  if( ! isRegular )
  {
    std::string f;
    if( sd[0].size() )
      f = x_last[1];
    else
      f = x_last[0];

    // could be a period with a wrong separator
    std::string sep;
    size_t pos=std::string::npos;

    bool isSep=false;
    for( size_t i=0 ; i < f.size() ; ++i )
    {
      if( !hdhC::isDigit(f[i]) )
      {
        if(isSep)
          return true;  // not a wrong separator, just anything

        isSep=true;
        sep=f[i];
        pos=i;
      }
    }

    if( pos < std::string::npos )
    {
      sd[0]=f.substr(0, pos) ;
      sd[1]=f.substr(pos+1) ;

      // Wrong separator for appendix clim or ave, found '_'
      std::string key("1_6b") ;
      if( notes->inq( key, pQA->fileStr) )
      {
        std::string capt("Wrong separation of filename's period's dates");
        capt += ", found ";
        capt += hdhC::tf_val(sep);

        (void) notes->operate(capt) ;
        notes->setCheckStatus(drsF, pQA->n_fail );
      }
    }
  }

  // in case of something really nasty
  if( !( hdhC::isDigit(sd[0]) && hdhC::isDigit(sd[1]) ) )
    return true;

  return false;
}

// class for comparing meta-data from the file to the
// "CMIP5 Model Output Requirements", but global attributes
CMOR::CMOR(QA* p)
{
  pQA = p;
  pExp = &p->qaExp;
  notes = p->notes;

  initDefaults();

  applyOptions(p->optStr);
}

void
CMOR::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "pEI" || split[0] == "parentExperimentID"
         || split[0] == "parent_experiment_id" )
     {
       if( split.size() == 2 )
       {
          parentExpID=split[1];
       }
       continue;
     }

     if( split[0] == "pER" || split[0] == "parentExperimentRIP"
         || split[0] == "parent_experiment_rip" )
     {
       if( split.size() == 2 )
       {
          parentExpRIP=split[1];
       }
       continue;
     }
  }

  return;
}

void
CMOR::checkEnsembleMemItem(std::string& rqName, std::string& attVal)
{
  std::string capt;
  std::string key;

  if( ! hdhC::isDigit(attVal) )
  {
    key = "2_5a";
    capt = n_global + hdhC::blank ;
    capt += hdhC::tf_att(rqName);
    capt += "must be integer, found ";
    capt += attVal;
  }
  else
  {
    int id=atoi(attVal.c_str());

    if( id == 0 && pExp->getFrequency() != "fx" )
    {
      key = "2_5b";
      capt = n_global + hdhC::blank ;
      capt += hdhC::tf_att(rqName);
      capt += "must be an integer > 0 for";
      capt += hdhC::tf_assign(pQA->n_frequency, pExp->getFrequency());
    }

    else if( id > 0 && pExp->getFrequency() == "fx" )
    {
      key = "2_5c";
      capt = n_global + hdhC::blank ;
      capt += hdhC::tf_att(rqName);
      capt += "must be equal 0 for frequency=<fx> ";
    }
  }

  if( capt.size() )
  {
    if( notes->inq( key, n_global) )
    {
      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return;
}

void
CMOR::checkForcing(std::vector<std::string>& vs_rqValue, std::string& aV)
{
  if( aV == hdhC::NA )
    return;

  Split x_aV(aV, ",");

  std::vector<std::string> vs_items(x_aV.getItems());
  vs_items = hdhC::unique(vs_items, hdhC::blank);

  std::string item;
  size_t p0, p1;

  // not a comma-sep list, but separated by blanks?
  if( vs_items.size() == 1 && aV.find(" ") < std::string::npos )
  {
      std::string key("2_6b");
      if( notes->inq( key, pQA->s_global ) )
      {
        std::string capt(pQA->s_global);
        capt += hdhC::blank;
        capt += hdhC::tf_att(hdhC::empty, n_forcing, aV);
        capt += "should be a comma separated list, found blanks";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV",  pQA->n_fail );
      }

      x_aV.setSeparator(' ');
      x_aV = aV;
      vs_items = x_aV.getItems() ;
      vs_items = hdhC::unique(vs_items, hdhC::blank);
  }

  for( size_t i=0 ; i < vs_items.size() ; ++i )
  {
    if( (p0=vs_items[i].find("(")) < std::string::npos )
    {
      if( p0 )
        item = vs_items[i].substr(0,p0);

      if( (p1=vs_items[i].find(")")) < std::string::npos )
        item += vs_items[i].substr(++p1);
    }
    else
      item=vs_items[i];

    // check
    if( ! hdhC::isAmong(item, vs_rqValue) )
    {
      std::string key("2_6a");
      if( notes->inq( key, pQA->s_global ) )
      {
        std::string capt(pQA->s_global);
        capt += hdhC::blank;
        capt += hdhC::tf_att(hdhC::empty, n_forcing, item);
        capt += "not among DRS-CV requested values";
        capt += hdhC::tf_val( hdhC::catStringVector(vs_rqValue)) ;

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV",  pQA->n_fail );

        break;
      }
    }
  }

  return;
}

bool
CMOR::checkMIP_table(InFile& in, VariableMetaData& vMD,
             std::vector<struct DimensionMetaData>& vs_f_DMD_entries)
{
   // return true for the very special case that a tracer
   // variable was not found in table sheet Omon, because
   // it is defined in Oyr

   if( !pExp->varReqTable.is() )  // no standard table
      return false;

   ReadLine ifs(pExp->varReqTable.getFile());

   if( ! ifs.isOpen() )
   {
      std::string key("7_4") ;

      if( notes->inq( key, vMD.var->name) )
      {
         std::string capt("could not open table.") ;
         capt += pExp->varReqTable.basename ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV", pQA->n_fail);
         pQA->setExitState( notes->getExitState() ) ;
      }
   }

  // headings for variables
  std::map<std::string, size_t> v_col;

  // read headings from the variable requirements table,
  std::string str0;

  readHeadline(ifs, vMD, str0, v_col);

  // find the table sheet, corresponding to the 2nd item
  // in the variable name. The name of the table sheet is
  // present in the first column and begins with "CMOR Table".
  // The remainder is the name.
  // Unfortunately, the variable requirement table is in a shape of a little
  // distance from being perfect.

  VariableMetaData tEntry(pQA);

  if( findVarReqTableEntry(ifs, str0, vMD, v_col, tEntry) )
  {
    // get properties of the table entry
      getMIPT_var(str0, tEntry, v_col); // normal; mostly
  }

  ifs.close();

  // file and table properties are compared to each other
  checkMIPT_var(vMD, tEntry, v_col);

  Split x_tDims(tEntry.attMap[n_CMOR_dimension]);

  for( size_t i=0 ; i < x_tDims.size() ; ++i )
  {
    // check basic properties between the file and
    // requests in the table.
;//    checkMIPT_dim(vs_dimSheet, vMD, d_col, x_tDims[i]);
  }

  // variable not found in the table.
  return false;
}

void
CMOR::checkMIPT_dim( std::vector<std::string>& vs_dimSheet,
  VariableMetaData& vMD,
  std::map<std::string, size_t>& col, std::string& CMORdim)
{
  // the CMOR-dimension column of the dim-sheet provides a
  // connection to the var entries of the other tables.
  struct DimensionMetaData t_DMD ;

  std::vector<std::string> vs_tOut;

  getDimSheetEntry(CMORdim, vs_dimSheet, col, t_DMD);

  // find corresponding var-representive name f the dim in the file
  size_t ix;
  for( ix=0 ; ix < pQA->pIn->varSz ; ++ix)
  {
    if( pQA->pIn->variable[ix].name == t_DMD.attMap[n_output_dim_name] )
      break;
  }

  if( ix == pQA->pIn->varSz )
  {
    std::string key("4_1a");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(hdhC::tf_assign(n_CMOR_dimension,CMORdim));
      capt += " is not represented in the file" ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    return;
  }

  // instance for a var-rep of a dim in the file
  DimensionMetaData f_DMD_entry;

  // meta-data of variables' dimensions
  if( getDimMetaData(*pQA->pIn, vMD, f_DMD_entry,
                     pQA->pIn->variable[ix].name, ix) )
    // compare findings from the file with those from the table
    checkMIPT_dimEntry(vMD, f_DMD_entry, t_DMD) ;

  return;
}

void
CMOR::checkMIPT_dimEntry(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // Do the dimensional meta-data found in the netCDF file
  // match those in the standard output table?

  if( t_DMD.attMap[n_output_dim_name] != f_DMD.attMap[n_output_dim_name] )
    checkMIPT_dim_outname(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_standard_name] != f_DMD.attMap[n_standard_name] )
    checkMIPT_dim_stdName(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_long_name] != f_DMD.attMap[n_long_name] )
    checkMIPT_dim_longName(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_axis] != f_DMD.attMap[n_axis] )
    checkMIPT_dim_axis(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_units] != f_DMD.attMap[n_units] )
    checkMIPT_dim_units(vMD, f_DMD, t_DMD);

  // nothing to check
//  if( t_DMD.attMap[n_index_axis] == "ok" )
//    checkMIPT_dim_indexAxis(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_coordinates].size() )
    checkMIPT_dim_coordsAtt(vMD, f_DMD, t_DMD);

  // the second condition takes especially into account
  // declaration of a variable across table sheets
  // (Omon, cf3hr, and cfSites)
  if( t_DMD.attMap[n_bounds_quest] == "yes" )
    checkMIPT_dim_boundsQuest(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_min].size() )
    checkMIPT_dim_validMin(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_valid_max].size() )
    checkMIPT_dim_validMax(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_type] != f_DMD.attMap[n_type] )
    checkMIPT_dim_type(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_positive].size() )
    checkMIPT_dim_positive(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_value].size() )
    checkMIPT_dim_value(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_values].size() )
    checkMIPT_dim_boundsValues(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_requested].size() )
    checkMIPT_dim_requested(vMD, f_DMD, t_DMD);

  if( t_DMD.attMap[n_bounds_requested].size() )
    checkMIPT_dim_boundsRequested(vMD, f_DMD, t_DMD);

  return ;
}

void
CMOR::checkMIPT_dim_axis(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4e");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_axis ));

    if( f_DMD.attMap[n_axis].size() )
    {
      capt += "no match with the CMOR table request, found";
      capt += hdhC::tf_val(f_DMD.attMap[n_axis]);
      capt += ", expected";
    }
    else
     capt += "missing, CMOR table requests";

    capt += hdhC::tf_val(t_DMD.attMap[n_axis]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_boundsQuest(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // standard table requires a boundary variable
  if( f_DMD.var->bounds.size() )
    return ;

  std::string key("4_4i");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "bounds?" ));
    capt += "missing bounds-variable as requested by the CMOR table" ;

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_boundsRequested(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
   checkWithTolerance(f_DMD, t_DMD, n_bounds_requested );
   return;
}

void
CMOR::checkMIPT_dim_boundsValues(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check dimensional values

  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;

  checkWithTolerance(f_DMD, t_DMD, n_bounds_values);

  return ;
}

void
CMOR::checkMIPT_dim_coordsAtt(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::vector<std::string> dimVar;
  dimVar.push_back("region");
  dimVar.push_back("passage");
  dimVar.push_back("type_description");

  std::vector<std::string> coAtt;
  coAtt.push_back("basin");
  coAtt.push_back("line");
  coAtt.push_back(n_type);

  std::string& t_DMD_coAtt   = t_DMD.attMap[n_coordinates] ;

  // note: the loop is mostly passed without action
  for(size_t ix=0 ; ix < coAtt.size() ; ++ix )
  {
    if( t_DMD_coAtt == coAtt[ix] )
    {
      // Is coordinates-att set in the corresponding variable?
      if( pQA->pIn->getVarIndex(dimVar[ix]) == -1 );
      {
        std::string key("4_4h");
        if( notes->inq( key, vMD.var->name) )
        {
          std::string capt("missing coordinates variable, expected");
          capt += hdhC::tf_val(dimVar[ix]);

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
          pQA->setExitState( notes->getExitState() ) ;
        }
      }

      break;
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_indexAxis(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  return;
}

void
CMOR::checkMIPT_dim_longName(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // generic level, thus user defined
  if( ! t_DMD.attMap[n_standard_name].size() )
    return;

  if( f_DMD.attMap.count(n_standard_name) &&
          t_DMD.attMap["proposed " + n_standard_name]
            == f_DMD.attMap[n_standard_name] )
    return;

  std::string key("4_4d");

  std::string& t_DMD_long_name = t_DMD.attMap[n_long_name] ;
  std::string& f_long_name = f_DMD.attMap[n_long_name] ;

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "long name" ));
    if( f_long_name.size() )
    {
      capt += "found";
      capt += hdhC::tf_val(f_long_name);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD_long_name) ;

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

bool
CMOR::checkMIPT_dim_outname(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string key("4_4b");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(
        f_DMD, t_DMD, "output dimension name" ));
    capt += "missing output dimensions name in the file";

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return true;
}

void
CMOR::checkMIPT_dim_positive(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  std::string& t_positive = t_DMD.attMap[n_positive] ;
  std::string& f_positive = f_DMD.attMap[n_positive] ;

  if( t_positive == f_positive )
    return;

  std::string key("4_4m");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_positive));
    capt += hdhC::tf_att(n_positive);
    capt += "in the file does not match the request in the CMOR table";
    if( f_DMD.attMap[n_positive].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_positive]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_positive);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_requested(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
   // There could be layers additionally
   // to the 17 mandatory ones for the dimension 'plevs'.
   // Thus, we have to ignore the optional ones.
   // for the comparisons between standard output table and file

  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_requested);
  else
  {
    size_t i=0;
    if( t_DMD.attMap[n_CMOR_dimension] == "plevs" )
      i=17;

    checkWithTolerance(f_DMD, t_DMD, n_requested, i );
  }

  return;
}

void
CMOR::checkMIPT_dim_stdName(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // dimensions which may have various standard names
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  std::string key("4_4c");

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, "standard name" ));
    capt += "not as the CMOR table requested";
    if( f_DMD.attMap[n_standard_name].size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_DMD.attMap[n_standard_name]);
    }

    capt += ", expected";
    capt += hdhC::tf_val(t_DMD.attMap[n_standard_name]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_type(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  if( f_DMD.attMap[n_type] == "char" && t_DMD.attMap[n_type] == "character" )
     return;

  std::string key("4_4l");
  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_type ));
    capt += "CMOR table requests,";
    capt += hdhC::tf_val(t_DMD.attMap[n_type]);
    capt += ", found";
    capt += hdhC::tf_val(f_DMD.attMap[n_type]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkMIPT_dim_units(
     VariableMetaData& vMD,
     struct DimensionMetaData& f_DMD,
     struct DimensionMetaData& t_DMD)
{
  // Special: units related to any dimension but 'time'

  // Do the dimensional meta-data found in the netCDF file
  // match those in the table (standard or project)?

  // one or more blanks are accepted
  std::string f_units( hdhC::unique(f_DMD.attMap[n_units], ' ') );
  std::string t_units( hdhC::unique(t_DMD.attMap[n_units], ' ') );

  // all time values should be positive, just test the first one
  if( (pQA->qaTime.firstTimeValue - pQA->qaTime.refTimeOffset) < 0 )
  {
    std::string key("3_9b");
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
      capt += " values should be positive";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  if( t_units == f_units )
    return;

  // dimensions which may have various units
  std::string& name = t_DMD.attMap[n_output_dim_name] ;

  if( name == "lev" || name == "loc" || name == "site" )
    return;

  if( t_units.find('?') < std::string::npos )
  {
    // time

    // Note: units of 'time' in standard table: 'days since ?',
    //       in the file: days since a specific date.

    if( (f_units.find("days since") < std::string::npos)
          != (t_units.find("days since") < std::string::npos) )
    {
      std::string key("3_9");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
        capt += "ill-formatted units, found";
        capt += hdhC::tf_val(f_units);
        capt += ", expected <days since ...>";

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExitState( notes->getExitState() ) ;
      }
    }

    return;
  }

  bool hasUnits=false;

  // index axis has no units
  if( t_DMD.attMap[n_index_axis] == "ok" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string& t_odn = t_DMD.attMap[n_output_dim_name] ;

  // no units
  if( t_odn == "basin" || t_odn == "line" || t_odn == "type")
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  if( t_odn == "site" )
  {
    if( f_units.size() )
      hasUnits=true;
    else
      return;
  }

  std::string key("3_10");

  if( hasUnits )
  {
    if( notes->inq( key, vMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
      capt += "expted dim-less, found " + hdhC::tf_assign(n_units, f_units) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    return;
  }

  // regular case: mismatch
  key = "4_4f" ;

  if( notes->inq( key, vMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_units ));
    capt += "CMOR table requests";
    capt += hdhC::tf_val(t_units);
    if( f_units.size() )
    {
      capt += ", found";
      capt += hdhC::tf_val(f_units);
    }

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return ;
}

void
CMOR::checkMIPT_dim_validMin(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name, 0, -1);

  if( ma.size() )
  {
    double min=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] < min )
        min=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_min]) );

    if( min < t_val )
    {
      std::string key("4_4j");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_min ));
        capt += "data minimum is lower than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(min));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_min]);

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExitState( notes->getExitState() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_validMax(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, f_DMD.var->name, 0, -1);

  if( ma.size() )
  {
    double max=ma[0];

    for(size_t i=1 ; i< ma.size() ; ++i )
      if( ma[i] > max )
        max=ma[i];

    double t_val( hdhC::string2Double( t_DMD.attMap[n_valid_max]) );

    if( max > t_val )
    {
      std::string key("4_4k");
      if( notes->inq( key, vMD.var->name) )
      {
        std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, n_valid_max ));
        capt += "data maximum is higher than requested by the CMOR table";
        capt += ", found";
        capt += hdhC::tf_val(hdhC::double2String(max));
        capt += ", expected";
        capt += hdhC::tf_val(t_DMD.attMap[n_valid_max]);

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
        pQA->setExitState( notes->getExitState() ) ;
      }
    }
  }

  return;
}

void
CMOR::checkMIPT_dim_value(
    VariableMetaData& vMD,
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD)
{
  // check values of limited dimensions
  if( f_DMD.var->isUnlimited() )
    return;


  std::string& odn = t_DMD.attMap[n_output_dim_name] ;

  if( odn == n_type || odn == "passage" || odn == "region" )
    checkStringValues(f_DMD, t_DMD, n_value );
  else
    checkWithTolerance(f_DMD, t_DMD, n_value);

  return ;
}

void
CMOR::checkMIPT_var(
    VariableMetaData& vMD,
    VariableMetaData& tEntry,
    std::map<std::string, size_t>& v_col)
{
  // Do the variable properties found in the netCDF file
  // match those in the MIP table?
  // Note: priority is not defined in the netCDF file.
  std::string t0;

  if( tEntry.attMap[n_cell_measures] != vMD.attMap[n_cell_measures] )
    checkMIPT_var_cellMeasures(vMD, tEntry);

  if( tEntry.attMap[n_cell_methods] != vMD.attMap[n_cell_methods] )
    checkMIPT_var_cellMethods(vMD, tEntry);

  if( tEntry.attMap[n_flag_meanings].size() )
    checkMIPT_var_flagMeanings(vMD, tEntry);

  if( tEntry.attMap[n_flag_values].size() )
    checkMIPT_var_flagValues(vMD, tEntry);

  if( tEntry.attMap[n_long_name] != vMD.attMap[n_long_name] )
    checkMIPT_var_longName(vMD, tEntry);

  if( tEntry.attMap[n_positive] != vMD.attMap[n_positive] )
    checkMIPT_var_positive(vMD, tEntry);

  // note that the variable in the file has no attribute realm, but a global one
  std::string f_realm = pQA->pIn->variable[pQA->pIn->varSz].getAttValue(n_realm);
  if( tEntry.attMap[n_realm] != f_realm )
    checkMIPT_var_realm(vMD, tEntry);

  if( tEntry.attMap[n_standard_name] != vMD.attMap[n_standard_name] )
    checkMIPT_var_stdName(vMD, tEntry);

  checkMIPT_var_type(vMD, tEntry);

  if( tEntry.attMap[n_units] != vMD.attMap[n_units] )
    checkMIPT_var_Units(vMD, tEntry);

  return ;
}

void
CMOR::checkMIPT_var_cellMeasures(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the format was checked in CF

  // get cm items
  std::vector<std::string> vs_cm_te(
      hdhC::itemise(tEntry.attMap[n_cell_measures], ":", "last") ) ;
  std::vector<std::string> vs_cm_ve(
      hdhC::itemise(vMD.attMap[n_cell_measures], ":", "last") ) ;

  bool is=true;
  size_t tSz = vs_cm_te.size();
  size_t vSz = vs_cm_ve.size();

  if( vSz == tSz )
  {
    is=false;

    for( size_t i=0 ; i < vSz ; ++ i )
    {
      size_t j;
      for( j=0 ; j < tSz ; ++ j )
        if( vs_cm_ve[i] == vs_cm_te[j] )
          break;

      if( j == tSz )
      {
        is=true;
        break;
      }
    }
  }

  if(is)
  {
    std::string key("3_3");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableId) ;

      std::string capt(QA_Exp::getCaptionIntroVar(
              currTable, vMD, n_cell_measures));

      if( vMD.attMap[n_cell_measures].size() )
      {
        capt += ", found" ;
        capt += hdhC::tf_val(vMD.attMap[n_cell_measures]) ;
      }
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      capt += ", expected";
      if( tEntry.attMap[n_cell_measures].size() )
        capt += hdhC::tf_val(tEntry.attMap[n_cell_measures]) ;
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_cellMethods(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the format was checked in CF

  // get cm items
  std::vector<std::string> vs_cm_te(
      hdhC::itemise(tEntry.attMap[n_cell_methods], ":", "last") ) ;
  std::vector<std::string> vs_cm_ve(
      hdhC::itemise(vMD.attMap[n_cell_methods], ":", "last") ) ;

  bool is=true;
  size_t tSz = vs_cm_te.size();
  size_t vSz = vs_cm_ve.size();

  if( vSz == tSz )
  {
    is=false;

    for( size_t i=0 ; i < vSz ; ++ i )
    {
      size_t j;
      for( j=0 ; j < tSz ; ++ j )
        if( vs_cm_ve[i] == vs_cm_te[j] )
          break;

      if( j == tSz )
      {
        is=true;
        break;
      }
    }
  }

  if(is)
  {
    std::string key("3_3");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableId) ;

      std::string capt(QA_Exp::getCaptionIntroVar(
              currTable, vMD, n_cell_methods));

      if( vMD.attMap[n_cell_methods].size() )
      {
        capt += ", found" ;
        capt += hdhC::tf_val(vMD.attMap[n_cell_methods]) ;
      }
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      capt += ", expected";
      if( tEntry.attMap[n_cell_methods].size() )
        capt += hdhC::tf_val(tEntry.attMap[n_cell_methods]) ;
      else
        capt += hdhC::tf_val(pQA->notAvailable) ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_flagMeanings(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_flagValues(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_frequency(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_longName(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{

  return ;
}

bool
CMOR::checkMIPT_var_positive(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return false;
}

void
CMOR::checkMIPT_var_realm(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_stdName(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkMIPT_var_type(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  // the standard table has type==real. Is it for
  // float only, or also for double? So, in case of real,
  // any non-int type is accepted
  bool isTblTypeReal =
      tEntry.attMap[n_type] == "real"
         || tEntry.attMap[n_type] == "float"
             || tEntry.attMap[n_type] == "double" ;
  bool isNcTypeReal =
      vMD.attMap[n_type] == "real"
         || vMD.attMap[n_type] == "float"
              || vMD.attMap[n_type] == "double" ;

  if( tEntry.attMap[n_type].size() == 0 && vMD.attMap[n_type].size() != 0 )
  {
    std::string key("4_11");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableId) ;

      std::string capt(QA_Exp::getCaptionIntroVar(currTable, vMD, n_type));
      capt += "check discarded, not found in table " ;
      capt += hdhC::tf_assign(QA::tableId);

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }
  else if( (isTblTypeReal && ! isNcTypeReal)
            || ( ! isTblTypeReal && isNcTypeReal) )
  {
    std::string key("4_8");

    if( notes->inq( key, vMD.var->name) )
    {
      std::string currTable(QA::tableId) ;

      std::string capt(QA_Exp::getCaptionIntroVar(currTable, vMD, n_type));
      capt += ", expected";

      if( tEntry.attMap[n_type].size() )
        capt += hdhC::tf_assign("type", tEntry.attMap[n_type]) ;
      else
        capt += " no type";

      capt += ", found" ;
      if( vMD.attMap[n_type].size() )
        capt += hdhC::tf_val(vMD.attMap[n_type]) ;
      else
        capt += "no type" ;


      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return ;
}

void
CMOR::checkMIPT_var_Units(
            VariableMetaData& vMD,
            VariableMetaData& tEntry)
{
  return ;
}

void
CMOR::checkRequestedAttributes(void)
{
  // check requested attributes of specific variables
  for(size_t i=0 ; i < pQA->drs_cv_table.varName.size() ; ++i)
  {
    // global attributes
    if( pQA->drs_cv_table.varName[i] == pQA->s_global
            && pQA->pIn->variable[pQA->pIn->varSz].name == "NC_GLOBAL" )
      ; //checkReqAtt_global();
    else
    {
      for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
      {
        // check for requested attributes of variables (whether coordinates var
        // or data var doesn't matter).
        if( pQA->drs_cv_table.varName[i] == pQA->pIn->variable[j].name )
        {
          checkReqAtt_variable(pQA->pIn->variable[j]);
          break;
        }
      }
    }
  }

  // checkSource();

/*
  // check attribute _FillValue and missing_value, if available
  std::string fV("_FillValue");
  std::string mV("missing_value");

  std::vector<std::string> fVmv;
  fVmv.push_back(fV);
  fVmv.push_back(mV);

  for( size_t j=0 ; j < pQA->pIn->varSz ; ++j )
  {
    Variable& var = pQA->pIn->variable[j] ;

    for( size_t l=0 ; l < 2 ; ++l )
    {
      int k;
      if( (k=var.getAttIndex(fVmv[l])) > -1 )
      {
        std::vector<float> aV;
        pQA->pIn->nc.getAttValues(aV, fVmv[l], var.name ) ;
        float rV = 1.E20 ;

        if( aV.size() == 0 || aV[0] != rV )
        {
          std::string key("3_11");

          if( notes->inq( key, var.name) )
          {
            std::string capt(
              hdhC::tf_att(var.name, fVmv[l], var.attValue[j][0])) ;
            capt += "does not match requested value=1.E20";

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV",  pQA->n_fail );
          }
        }
      }
    }
  }

*/

  return ;
}

void
CMOR::checkReqAtt_global(void)
{
  // check requested attributes
  DRS_CV_Table& drs = pQA->drs_cv_table;

  Variable& glob = pQA->pIn->variable[pQA->pIn->varSz] ;

  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == pQA->s_global )
      break;

  if( ix == drs.varName.size() )
    return;

  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // missing requested global attribute
    if( ! glob.isValidAtt(rqName) )
    {
       std::string key("2_3");

       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("requested ");
         capt += pQA->s_global;
         capt += hdhC::blank;
         capt += hdhC::tf_att(rqName);
         capt += "is missing" ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );

         continue;
       }
    }

    // requested attribute expects a requested value
    if( rqValue.size() )  // a value is requested
    {
      std::string aV = glob.getAttValue(rqName) ;

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, pQA->s_global) )
        {
           std::string capt(pQA->s_global);
           capt += hdhC::blank;
           capt += hdhC::tf_att(rqName, hdhC::no_blank);
           capt=" missing requested value=";
           capt += rqValue;

           if( rqName == n_forcing )
           {
             capt += ", expected at least";
             capt += hdhC::tf_assign(n_forcing, "N/A") ;
           }

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }
       else
       {
          // note that several alternatives may be acceptable
          Split x_rqValue(rqValue, "|");
          std::vector<std::string> vs_rqValue(x_rqValue.getItems());
          vs_rqValue = hdhC::unique(vs_rqValue, hdhC::blank);

          if( rqName == n_forcing )
            checkForcing(vs_rqValue, aV);
          else if( rqName == "initialization_method"
                      || rqName == "physics_version_method"
                         || rqName == "realization" )
            checkEnsembleMemItem(rqName, aV) ;

          else if( x_rqValue[0] == "YYYY-MM-DDTHH:MM:SSZ" )
          {
            // a valid date format may have T substituted by a blank
            // and Z omitted or the latter be lower case.
            bool is=false;
            if( aV.size() > 18 )
            {
              if( aV[4] == '-' && aV[7] == '-' )
              {
                  if( aV[10] == 'T' || aV[10] == ' ' )
                  {
                    if( aV[13] == ':' && aV[16] == ':' )
                    {
                      if( aV.size() == 20 )
                      {
                        if( !(aV[19] == 'Z' || aV[19] == 'z' ) )
                          is=true;
                      }

                      if( ! hdhC::isDigit( aV.substr(0, 4) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(5, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(8, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(11, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(14, 2) ) )
                        is=true ;
                      else if( ! hdhC::isDigit( aV.substr(17, 2) ) )
                        is=true ;
                    }
                    else
                      is=true;
                  }
                  else
                    is=true;
              }
              else
                is=true;
            }
            else
              is=true;

            if( is )
            {
              std::string key("2_4");
              if( notes->inq( key, pQA->s_global) )
              {
                std::string capt(pQA->s_global);
                capt += hdhC::blank;
                capt += hdhC::tf_att(rqName);
                capt += "does not comply with YYYY-MM-DDThh:mm:ssZ, found: " ;
                capt += aV ;

                (void) notes->operate(capt) ;
                notes->setCheckStatus("CV",  pQA->n_fail );
              }
            }
          }

          else if( !hdhC::isAmong(aV, vs_rqValue) )
          {
            // there is a value. is it the requested one?
            std::string key("2_2");
            if( notes->inq( key, pQA->s_global ) )
            {
              std::string capt(pQA->s_global);
              capt += hdhC::blank;
              capt += hdhC::tf_att(hdhC::empty, rqName, aV);
              capt += "does not match requested value";
              capt += hdhC::tf_val(rqValue) ;

              (void) notes->operate(capt) ;
              notes->setCheckStatus("CV",  pQA->n_fail );

              continue;
            }
          }
       }
    }
  } // end of for-loop

  return;
}

void
CMOR::checkReqAtt_variable(Variable &var)
{
  // vName := name of an existing variable (already tested)
  // ix    := index of InnFile.variable[ix]

/*
  DRS_CV_Table& drs = pQA->drs_cv_table;

  std::string &vName = var.name;

  // check required attributes
  size_t ix;
  for( ix=0 ; ix < drs.varName.size() ; ++ix )
    if( drs.varName[ix] == vName )
      break;

  // find required attributes missing in the file
  for( size_t k=0 ; k < drs.attName[ix].size(); ++k )
  {
    std::string& rqName  = drs.attName[ix][k] ;
    std::string& rqValue = drs.attValue[ix][k] ;

    // find corresponding att_name of given vName in the file
    int jx=var.getAttIndex(rqName);

    // special: values may be available for cloud amounts

    // missing required attribute
    if( jx == -1 && rqName != "values" )
    {
       std::string key("2_1");

       if( notes->inq( key, vName) )
       {
         std::string capt(hdhC::tf_att(vName,rqName));
         capt += "is missing";

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );

         continue;
       }
    }

    std::string &aN = var.attName[jx] ;

    // required attribute expects a value (not for cloud amounts)
    if( rqValue.size() )  // a value is required
    {
      if( ! isCloudAmount && rqName == "values" )
         continue;

      std::string aV( var.attValue[jx][0] ) ;
      for( size_t i=1 ; i < var.attValue[jx].size() ; ++i )
      {
        aV += " " ;
        aV += var.attValue[jx][i] ;
      }

      if( aV.size() == 0 )
      {
        std::string key("2_2");
        if( notes->inq( key, vName) )
        {
           std::string capt(hdhC::tf_att(vName, aN, hdhC::colon));
           capt="missing required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }

       // there is a value. is it the required one?
       else if( aV != rqValue )
       {
         bool is=true;
         if( rqName == pQA->n_long_name)
         {
           // mismatch tolerated, because the table does not agree with CMIP5
           if( rqValue == "pressure" && aV == "pressure level" )
             is=false;
           if( rqValue == "pressure level" && aV == "pressure" )
             is=false;
         }
         else if( (rqName == pQA->n_positive) || (rqName == pQA->n_axis) )
         {
           // case insensitive
           if( rqValue == hdhC::Lower()(aV) )
             is=false;
         }

         std::string key("2_3");
         if( is &&  notes->inq( key, vName ) )
         {
           std::string capt(hdhC::tf_att(vName, aN, aV));
           capt += "does not match required value=" ;
           capt += rqValue;

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV",  pQA->n_fail );

           continue;
         }
       }
    }
  } // end of for-loop
*/

  return;
}

void
CMOR::checkSource(void)
{
  // global attribute 'source'
  Variable& glob = pQA->pIn->variable[pQA->pIn->varSz] ;

  std::string n_source("source");

  Split x_colon(glob.getAttValue(n_source),";");

  // note: existence is checked elsewhere
  if( x_colon.size() == 0 )
    return;

  // some words
  std::string n_model_id("model_id");

  std::vector<std::string> vs_dscr;
  vs_dscr.push_back("atmosphere:");
  vs_dscr.push_back("ocean:");
  vs_dscr.push_back("sea ice:");
  vs_dscr.push_back("land:");

  Split x_word(x_colon[0]);
  x_word.setProtector("()",true);

  Split x_brackets;
  x_brackets.setSeparator("()", true);

  Split x_brackets_items;

  for( size_t i=0; i < x_colon.size() ; ++i )
  {
    size_t j=0;
    x_word = x_colon[i] ;

    if( i==0 )
    {
      std::string model_id(glob.getAttValue(n_model_id)) ;

      if( x_word[j].size() == 0 )
      {
        std::string key("2_7a");
        if( notes->inq(key) )
        {
          std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
          capt += "The 1st item should be the model_id attribute";

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
        }
      }

      else
      {
        if( model_id.size() != x_word[j].size() )
        {
          std::string key("2_7b");
          if( notes->inq(key) )
          {
            std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
            capt += "The model_id does not match, found";
            capt += hdhC::tf_val(x_word[j]) ;
            capt += ", expected";
            capt += hdhC::tf_val(model_id) ;

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV", pQA->n_fail);
          }

          ++j;
        }

        if( x_word[j].size() > 1 )
        {
          if( ! hdhC::isDigit(x_word[1]) )
          {
            std::string key("2_7c");
            if( notes->inq(key) )
            {
              std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
              capt += "The 2nd item should be a year in digits";

              (void) notes->operate(capt) ;
              notes->setCheckStatus("CV", pQA->n_fail);
            }
          }

          ++j;
        }
      }
    }

    // look for descriptors
    size_t dscr_ix;
    for( dscr_ix=0 ; dscr_ix < vs_dscr.size() ; ++dscr_ix )
      if( vs_dscr[dscr_ix] == x_word[j] )
        break;

    if( dscr_ix < vs_dscr.size() )
    {
      // model_name is required before ()-term
      if( x_word[j][0] == '(' )
      {
        std::string key("2_7d");
        if( notes->inq(key) )
        {
          std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
          capt += "The descriptor";
          capt += hdhC::tf_val(vs_dscr[dscr_ix]) ;
          capt += " should be followed by a model_name";

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV", pQA->n_fail);
        }
      }
      else
        ++j;
    }

    // the term in brackets
    bool foundBrackets=false;

    for( ; j < x_word.size() ; ++j )
    {
//      size_t last = x_word[j].size() - 1;

      x_brackets = x_colon[i];

//      if( x_word[j][0] == '(' && x_word[j][last] == ')' )
      if( x_brackets.size() > 1 )
      {
        foundBrackets=true;
//        x_brackets_items = x_word[j].substr(1,last-1);
        x_brackets_items = x_brackets[1];

        bool isA = (x_brackets_items.size() == 1 && dscr_ix != 2 )
                    ? true : false ;   // sea ice: or land:

        if( !isA )
          isA = x_brackets_items.size() == 2 ? false : true ;

        if(isA)
        {
          std::string key("2_7f");
          if( notes->inq(key) )
          {
            std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
            capt += "faulty term (<technical_name>, <resolution_and_levels>), found";
            capt += hdhC::tf_val(x_brackets_items.getStr()) ;

            (void) notes->operate(capt) ;
            notes->setCheckStatus("CV", pQA->n_fail);
          }

          break;
        }
      }
    }

    if( dscr_ix < vs_dscr.size() && !foundBrackets )
    {
      std::string key("2_7e");
      if( notes->inq(key) )
      {
        std::string capt(hdhC::tf_att(n_global, n_source, hdhC::colon));
        capt += "Missing bracketed term in ";
        capt += x_colon[i];

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CV", pQA->n_fail);
      }
    }
  }

  return;
}

void
CMOR::checkStringValues( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  std::vector<std::string> vs_values;

  pQA->pIn->nc.getData(vs_values, f_DMD.var->name);

  bool is=true;
  if( t_DMD.attMap[CMOR::n_CMOR_dimension] == "plevs")
  {
      // There are 17 mandatory levels and up to 6 additional levels
      // requested of models with sufficient resolution in the stratosphere.
      if( vs_values.size() > static_cast<size_t>(23) )
      {
         std::string key("4_4p");
         if( notes->inq( key, f_DMD.var->name) )
         {
           std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
           capt += "provision of more than the requested max(17+6) levels, found";
           capt += hdhC::tf_val(hdhC::double2String(vs_values.size()));

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV", pQA->n_fail);
           pQA->setExitState( notes->getExitState() ) ;
         }

         is=false;
      }
  }

  if( is && vs_values.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(vs_values.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    return;
  }

  size_t i;
  for( i=0 ; i < vs_values.size() ; ++i )
    vs_values[i] = hdhC::stripSides(vs_values[i]) ;

  for( i=0 ; i < x_tVal.size() ; ++i )
    if( !hdhC::isAmong(x_tVal[i], vs_values) )
      break;

  if( i == x_tVal.size() )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "mismatch of data values between file and table, expected also";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
CMOR::checkWithTolerance( struct DimensionMetaData& f_DMD,
  struct DimensionMetaData& t_DMD,
  std::string& cName, size_t maxSz )
{
  Split x_tVal( t_DMD.attMap[cName]) ;

  std::string effName(f_DMD.var->name);

  if( cName == n_bounds_values || cName == n_bounds_requested )
  {
     // switch from current dim-var to corresponding dim-bnd-var
     if(f_DMD.var->bounds.size())
         effName=f_DMD.var->bounds;
  }

  MtrxArr<double> ma;
  pQA->pIn->nc.getData(ma, effName, 0, -1);

  bool is=true;
  if( t_DMD.attMap[CMOR::n_CMOR_dimension] == "plevs")
  {
      // There are 17 mandatory levels and up to 6 additional levels
      // requested of models with sufficient resolution in the stratosphere.
      if( ma.size() > static_cast<size_t>(23) )
      {
         std::string key("4_4p");
         if( notes->inq( key, f_DMD.var->name) )
         {
           std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
           capt += " max. no. of levels=(17+6), found";
           capt += hdhC::tf_val(hdhC::double2String(ma.size()));

           (void) notes->operate(capt) ;
           notes->setCheckStatus("CV", pQA->n_fail);
           pQA->setExitState( notes->getExitState() ) ;
         }

         is=false;
      }
  }

  if( is && ma.size() != x_tVal.size() )
  {
    std::string key("4_4");

    if( cName == n_value )
      key += 'n';
    else if( cName == n_bounds_values )
      key += 'o';
    else if( cName == n_requested )
      key += 'p';
    else if( cName == n_bounds_requested )
      key += 'q';

    if( notes->inq( key, f_DMD.var->name) )
    {
      std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
      capt += "mismatch of number of data values, found";
      capt += hdhC::tf_val(hdhC::double2String(ma.size()));
      capt += " items in the file and ";
      capt += hdhC::tf_assign(cName, hdhC::double2String(x_tVal.size()));
      capt += " in the table";

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    return;
  }

  double tolVal=0;
  if( t_DMD.attMap[n_tol_on_requests].size());
    tolVal = hdhC::string2Double(t_DMD.attMap[n_tol_on_requests]);

  if( !maxSz )
    maxSz = x_tVal.size();

  std::vector<double> tolMin;
  std::vector<double> tolMax;

  for( size_t i=0 ; i < maxSz ; ++i )
  {
    tolMin.push_back(ma[i] - tolVal);
    tolMax.push_back(ma[i] + tolVal);
  }

  // convert table values to double, which is always possible for CMIP5
  size_t i;
  for( i=0 ; i < maxSz ; ++i )
  {
    double tVal = hdhC::string2Double(x_tVal[i]) ;
    if( !( (tolMin[i] <= tVal) && (tVal <= tolMax[i]) ) )
      break;
  }

  if( i == maxSz )
    return;

  std::string key("4_4");

  if( cName == n_value )
    key += 'n';
  else if( cName == n_bounds_values )
    key += 'o';
  else if( cName == n_requested )
    key += 'p';
  else if( cName == n_bounds_requested )
    key += 'q';

  if( notes->inq( key, f_DMD.var->name) )
  {
    std::string capt(QA_Exp::getCaptionIntroDim(f_DMD, t_DMD, cName ));
    capt += "Mismatch of data values between file and table, found";
    capt += hdhC::tf_val(hdhC::double2String(ma[i]));
    capt += ", expected ";
    capt += hdhC::tf_assign(cName, x_tVal[i]);

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

bool
CMOR::findVarReqTableEntry(ReadLine& ifs, std::string& str0,
   VariableMetaData& vMD,
   std::map<std::string, size_t>& col, VariableMetaData& tEntry)
{
   // return true: entry is not the one we look for.
   Split x_line;
   x_line.setSeparator('|');
   x_line.enableEmptyItems();

   size_t colIx=col[CMOR::n_varName];

   while( ! ifs.getLine(str0) )
   {
     // Get the string for the MIP-sub-table corresponding to
     // time table entries read by parseTimeTable().
//     if( str0.substr(0,13) == "In CMOR Table" )
//     {
//       getVarReqTableSubSheetName(str0, vMD) ;
//       continue;
//     }

     x_line=str0;

     if( x_line.size() > colIx && x_line[colIx] == vMD.var->name )
       return true; // found an entry and the shape matches.
   }

   return false;
}

bool
CMOR::getDimMetaData(InFile& in,
      VariableMetaData& vMD,
      struct DimensionMetaData& f_DMD,
      std::string dName, size_t ix)
{
  // return 0:
  // collect dimensional meta-data into a struct.

  // the corresponding variable
  f_DMD.var = &pQA->pIn->variable[ix] ;

  // pre-set
  f_DMD.attMap[n_output_dim_name]=dName;
  f_DMD.attMap[n_coordinates]=dName;

  // get the var type
  f_DMD.attMap[n_type] = in.nc.getVarTypeStr(dName);

  // get the attributes

  for( size_t j=0 ; j < f_DMD.var->attName.size() ; ++j)
  {
    std::string& aName  = f_DMD.var->attName[j];
    std::string& aValue = f_DMD.var->attValue[j][0];

    if( aName == n_axis )
      f_DMD.attMap[n_axis] = aValue ;
    else if( aName == n_long_name )
    {
      f_DMD.attMap[n_long_name] = hdhC::unique(aValue, ' ') ;
      if( notes->inq( "7_14", vMD.var->name, "INQ_ONLY") )
        f_DMD.attMap[n_long_name] = hdhC::Lower()(f_DMD.attMap[n_long_name]) ;
    }
    else if( aName == n_standard_name )
      f_DMD.attMap[n_standard_name] = aValue ;
    else if( aName == n_units )
      f_DMD.attMap[n_units] = aValue ;
    else if( aName == n_positive )
      f_DMD.attMap[n_positive] = aValue ;
  }

  // determine the checksum of limited var-presentations of dim
  if( ! f_DMD.var->checksum && ! f_DMD.var->isUnlimited() )
  {
    if( f_DMD.var->type == NC_CHAR )
    {
      std::vector<std::string> vs;
      in.nc.getData(vs, dName);

      bool reset=true;  // is set to false during the first call
      for(size_t i=0 ; i < vs.size() ; ++i)
      {
        vs[i] = hdhC::stripSides(vs[i]);
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(vs[i], &reset) ;
      }
    }
    else
    {
      MtrxArr<double> mv;
      in.nc.getData(mv, dName, 0, -1);

      bool reset=true;
      for( size_t i=0 ; i < mv.size() ; ++i )
        f_DMD.var->checksum = hdhC::fletcher32_cmip5(mv[i], &reset) ;
    }
  }

  return dName.size() ? true : false;
}

void
CMOR::getDimSheetEntry(std::string& CMOR_dim,
   std::vector<std::string>& vs_dimSheet,
   std::map<std::string, size_t>& col,
   DimensionMetaData& t_DMD, std::string column)
{
   Split x_line;
   x_line.setSeparator(',');
   x_line.enableEmptyItems();

   // look for a matching entry in the dimension sheet
   for( size_t i=0 ; i < vs_dimSheet.size() ; ++i )
   {
     x_line=vs_dimSheet[i];

     // is it in the table?
     if( x_line[col[n_CMOR_dimension]] == CMOR_dim )
     {
       if( column.size() )
       {
          t_DMD.attMap[column]=x_line[col[column]];
          return ;
       }

       t_DMD.attMap[n_CMOR_dimension]=x_line[col[n_CMOR_dimension]];

       // is there a mapping to a name in column 'coords_attr'?
       if( x_line[col[n_coordinates]].size() )
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_coordinates]];
       else
         t_DMD.attMap[n_output_dim_name]  = x_line[col[n_output_dim_name]];

       t_DMD.attMap[n_standard_name] =x_line[col[n_standard_name]];

       t_DMD.attMap[n_long_name]
          = hdhC::unique(x_line[col[n_long_name]], ' ');
       if( notes->inq( "7_14", "", "INQ_ONLY") )
         t_DMD.attMap[n_long_name] = hdhC::Lower()(t_DMD.attMap[n_long_name]) ;

       t_DMD.attMap[n_axis]             = x_line[col[n_axis]];
       t_DMD.attMap[n_units]            = hdhC::unique(x_line[col[n_units]], ' ');
       t_DMD.attMap[n_index_axis]       = x_line[col[n_index_axis]];
       t_DMD.attMap[n_coordinates]      = x_line[col[n_coordinates]];
       t_DMD.attMap[n_bounds_quest]     = x_line[col[n_bounds_quest]];
       t_DMD.attMap[n_valid_min]        = x_line[col[n_valid_min]];
       t_DMD.attMap[n_valid_max]        = x_line[col[n_valid_max]];
       t_DMD.attMap[n_type]             = x_line[col[n_type]];
       t_DMD.attMap[n_positive]         = x_line[col[n_positive]];
       t_DMD.attMap[n_value]            = x_line[col[n_value]];
       t_DMD.attMap[n_bounds_values]    = x_line[col[n_bounds_values]];
       t_DMD.attMap[n_requested]        = x_line[col[n_requested]];
       t_DMD.attMap[n_bounds_requested] = x_line[col[n_bounds_requested]];

       t_DMD.attMap[n_tol_on_requests]  = hdhC::empty;
       std::string& str0 = x_line[col[n_tol_on_requests]] ;
       if( str0.substr(0, n_tol_on_requests.size()) == n_tol_on_requests )
          t_DMD.attMap[n_tol_on_requests]=x_line[col[n_tol_on_requests]];
     }
   }

  return;
}

void
CMOR::getMIPT_var(std::string& entry,
    VariableMetaData& tEntry,
    std::map<std::string, size_t>& v_col)
{
  Split x_line;
  x_line.setSeparator('|');
  x_line.enableEmptyItems();
  x_line = entry;

  std::vector<std::string*> vs_p;

  vs_p.push_back(&n_CMOR_varName);
  vs_p.push_back(&n_priority);
  vs_p.push_back(&n_long_name);
  vs_p.push_back(&n_standard_name);
  vs_p.push_back(&n_realm);
  vs_p.push_back(&n_frequency);
  vs_p.push_back(&n_cell_measures);
  vs_p.push_back(&n_flag_meanings);
  vs_p.push_back(&n_flag_values);
  vs_p.push_back(&n_cell_methods);
  vs_p.push_back(&n_positive);
  vs_p.push_back(&n_type);
  vs_p.push_back(&n_CMOR_dimension);

  for( size_t i=0 ; i < vs_p.size() ; ++i)
  {
    std::string& s0 = *(vs_p[i]);
    size_t ix = v_col[s0] ;

    if( ix || s0 == n_priority )
    {
      tEntry.attMap[s0] = hdhC::unique( x_line[ix], ' ' );

      if( s0 == n_units )
      {
        tEntry.var->isUnitsDefined = tEntry.var->units.size() ? true : false;
      }
      else if( s0 == n_long_name )
      {
        if( notes->inq( "7_14", "", "INQ_ONLY") )
          tEntry.attMap[n_long_name+"_insens"]
            = hdhC::Lower()(tEntry.attMap[n_long_name]) ;
      }
    }
    else
      tEntry.attMap[s0] = hdhC::empty;
  }

  return ;
}

void
CMOR::initDefaults(void)
{
  ;

  return;
}

bool
CMOR::readHeadline(ReadLine& ifs,
   VariableMetaData& vMD, std::string& line, std::map<std::string, size_t>& v_col)
{
   // find the captions for table sheets dims and for the variables

   // read the heading of the table sheet for the dimensions.
   // The table sheet is identified by the first column.
   if( ifs.getLine(line) )
     return true;

   Split x_line;
   x_line.setSeparator('|');
   x_line.enableEmptyItems();

   x_line=line;

   // now, the variable's columns
   for( size_t v=0 ; v < x_line.size() ; ++v)
   {
     if( x_line[v] == "Priority" )
       v_col[n_priority] = v;
     else if( x_line[v] == "Long name" )
       v_col[n_long_name] = v;
     else if( x_line[v] == "units" )
       v_col[n_units] = v;
     else if( x_line[v] == "description" )
       ;
     else if( x_line[v] == "comment" )
       ;
     else if( x_line[v] == "Variable Name" )
       v_col[n_varName] = v;
     else if( x_line[v] == "CF Standard Name" )
       v_col[n_standard_name] = v;
     else if( x_line[v] == n_cell_methods )
       v_col[n_cell_methods] = v;
     else if( x_line[v] == n_positive )
       v_col[n_positive] = v;
     else if( x_line[v] == n_type )
       v_col[n_type] = v;
     else if( x_line[v] == "dimensions" )
       v_col[n_CMOR_dimension] = v;
     else if( x_line[v] == "CMOR Name" )
       v_col[n_CMOR_varName] = v;
     else if( x_line[v] == "modeling_realm" )
       v_col[n_realm] = v;
     else if( x_line[v] == "frequency" )
       v_col[n_frequency] = v;
     else if( x_line[v] == n_cell_measures )
       v_col[n_cell_measures] = v;

/*
     else if( x_line[v] == "valid min" )
       v_col[n_valid_min] = v;
      else if( x_line[v] == "valid max" )
       v_col[n_valid_max] = v;
*/
   }

   return true;
}

void
CMOR::run(InFile& in, VariableMetaData& vMD)
{
  // Matching the ncfile inherent meta-data against pre-defined
  // CMOR tables.

  // in fact only global attributes
  checkRequestedAttributes();

  // Meta data of variables from file or table are stored in struct varMeDa.
  // Similar for dimMeDa for the associated dimensions.

  std::vector<struct DimensionMetaData> vs_f_DMD_entries;

  // Scan through the standard output table, respectivels variable requests.
  if( QA::tableId.size() && ! vMD.var->isExcluded )
  {
    checkMIP_table(in, vMD, vs_f_DMD_entries) ;
  }

  return;
}

void
CMOR::bufTableSheets(VariableMetaData& vMD)
{
/*
  if( tableIdBuf.size() )
  {
    if( QA::tableId == "Omon" )
      QA::tableIdSub = "Marine Bioge" ;
    else if( QA::tableId == "cf3hr" )
    {
      tableIdSub.clear() ;

      if( tableIdBuf.size() == 2 )
        vMD.attMap[n_cell_methods]=tableIdBuf[1];
    }
    else if( QA::tableId == "cfSites" )
    {
      QA::tableIdSub = "CFMIP 3-ho" ;

      if( tableIdBuf.size() == 2 )
        vMD.attMap[n_cell_methods]=tableIdBuf[1];
    }
  }

  else
  {
*/

  if( QA::tableId == "Omon" )
  {
    tableIdBuf.push_back("Oyr") ;
    tableIdBuf.push_back(QA::tableId) ;
  }
  else if( QA::tableId == "cf3hr" )
  {
    tableIdBuf.push_back(QA::tableId) ;
    QA::tableId = "Amon" ;
    tableIdBuf.push_back(vMD.attMap[n_cell_methods]);
    vMD.attMap[n_cell_methods]="time: point";
  }
  else if( QA::tableId == "cfSites" )
  {
    tableIdBuf.push_back(QA::tableId) ;
    QA::tableId = "Amon" ;
    tableIdBuf.push_back(vMD.attMap[n_cell_methods]);
    vMD.attMap[n_cell_methods]="time: point";
  }
  else
    tableIdBuf.push_back(QA::tableId) ;

//  }

  return;
}

// class with project specific purpose
QA_Exp::QA_Exp()
{
  initDefaults();
}

void
QA_Exp::applyOptions(std::vector<std::string>& optStr)
{
  for( size_t i=0 ; i < optStr.size() ; ++i)
  {
     Split split(optStr[i], "=");

     if( split[0] == "eA" || split[0] == "excludedAttribute"
         || split[0] == "excluded_attribute" )
     {
       Split cvs(split[1],",");
       for( size_t i=0 ; i < cvs.size() ; ++i )
         excludedAttribute.push_back(cvs[i]);
       continue;
     }

     if( split[0] == "tVR"
          || split[0] == "table_variable_requirements" )
     {
       if( split.size() == 2 )
          varReqTable.setPath(split[1]);
       continue;
     }

     if( split[0] == "uS" || split[0] == "useStrict"
            || split[0] == "use_strict" )
     {
          isUseStrict=true ;
          pQA->setCheckMode("meta");
     }
     continue;
   }

   // apply a general path which could have also been provided by setTablePath()
   if( varReqTable.path.size() )
      varReqTable.setPath(pQA->tablePath + "/" + varReqTable.path);

   return;
}

void
QA_Exp::checkDataVarNum(void)
{
  // each file must contain only a single output field from a single simulation
  if( pQA->pIn->dataVarIndex.size() == 1 )
    return;

  if( pQA->pIn->dataVarIndex.size() == 0 )
  {
    if( fVarname.size() )
    {
      // unfortunately, no data-variable was found previously
      for( size_t i=0 ; i < pQA->pIn->varSz ; ++i )
      {
        if( pQA->pIn->variable[i].name == fVarname )
        {
          // correction
          pQA->pIn->dataVarIndex.push_back(i);
          pQA->pIn->variable[i].isDATA=true;
          return;
        }
      }
    }

    std::string key("9_1b");
    if( notes->inq( key) )
    {
      std::string capt("no data variable present") ;

      (void) notes->operate(capt) ;
      notes->setCheckStatus("CV", pQA->n_fail);
      pQA->setExitState( notes->getExitState() ) ;
    }

    return;
  }

  std::string key("9_1a");
  if( notes->inq( key) )
  {
    std::string capt("multiple variables, found: ") ;

    for( size_t i=0 ; i < pQA->pIn->dataVarIndex.size() ; ++i)
    {
      if(i)
        capt += ", ";

      capt += pQA->pIn->variable[pQA->pIn->dataVarIndex[i]].name;
    }

    (void) notes->operate(capt) ;
    notes->setCheckStatus("CV", pQA->n_fail);
    pQA->setExitState( notes->getExitState() ) ;
  }

  return;
}

void
QA_Exp::checkMetaData(InFile& in)
{
  // check basic properties between this file and
  // requests in the table. When a MIP-subTable is empty, use the one
  // from the previous instance.
  CMOR cmor(pQA);

  for( size_t i=0 ; i < varMeDa.size() ; ++i )
  {
    if( varMeDa[i].var->isDATA )
    {
      cmor.run(in, varMeDa[i] );
    }
  }

  // inquire passing the meta-data check
  int ev;
  if( (ev = notes->getExitState()) > 1 )
    pQA->setExitState( ev );

  return;
}

void
QA_Exp::createVarMetaData(void)
{
  // create instances of VariableMetaData. These have been identified
  // previously at the opening of the nc-file and marked as
  // Variable::VariableMeta(Base)::isDATA == true. The index
  // of identified targets is stored in the InFile::dataVarIndex vector.

  //there should be only present a single data variable
  checkDataVarNum();

  for( size_t i=0 ; i< pQA->pIn->dataVarIndex.size() ; ++i )
  {
    Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[i]];

    pushBackVarMeDa( &var );  //push next instance
  }

  if( pQA->pIn->dataVarIndex.size() > 1 )
  {
     std::string key("9_1");
     if( notes->inq( key, pQA->fileStr) )
     {
       std::string capt("multiple data variables are present, found ");

       for( size_t j=0 ; j < pQA->pIn->dataVarIndex.size() ; ++j )
       {
         Variable &var = pQA->pIn->variable[pQA->pIn->dataVarIndex[j]];
         if(j)
           capt += ", ";

         capt += var.name;
       }

       (void) notes->operate(capt) ;
       notes->setCheckStatus("CV",  pQA->n_fail );
     }
  }

  return;
}

std::string
QA_Exp::getCaptionIntroDim(
    struct DimensionMetaData& f_DMD,
    struct DimensionMetaData& t_DMD, std::string att)
{
  std::string& t_DMD_outname = t_DMD.attMap[CMOR::n_output_dim_name] ;

  std::string intro("table=");
  intro += QA::tableId + ", var(file)=";
  intro += f_DMD.var->name + ", CMOR-dim=";
  intro += t_DMD.attMap[CMOR::n_CMOR_dimension] ;

  if( t_DMD_outname == "basin" )
    intro += ", region";
  if( t_DMD_outname == "line" )
    intro += ", passage";
  if( t_DMD_outname == CMOR::n_type )
    intro += ", type_description";

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
}

std::string
QA_Exp::getCaptionIntroVar( std::string table,
    struct VariableMetaData& f_VMD, std::string att)
{
  std::string intro("table=");
  intro += table + ", var=";
  intro += f_VMD.var->name ;

  if( att.size() )
    intro += ", " + att ;

  intro += ": ";
  return intro;
}

std::string
QA_Exp::getFrequency(void)
{
  if( frequency.size() )
    return frequency;  // already known

  // get frequency from attribute (it is required)
  frequency = pQA->pIn->nc.getAttString("frequency") ;

  if( frequency.size() )
    return frequency;  // no frequency provided

  // not found, but error issue is handled elsewhere

  // try the filename
  std::string f( pQA->pIn->file.basename );

  Split splt(f, "_");

  // the second term denotes the mip table for CMIP5
  std::string mip_f = splt[1];

  // now, try also global att 'table_id'
  Split mip_a( pQA->pIn->nc.getAttString(CMOR::n_table_id) ) ;

  if( mip_a.size() > 1 )
  {
    if( mip_a[1] != mip_f && mip_a[1].size() )
      mip_f = mip_a[1]; // annotation issue?

    // convert mip table --> frequency
    if( mip_f.substr(mip_f.size()-2) == "yr" )
      frequency = "yr" ;
    else if( mip_f.substr(mip_f.size()-3) == "mon" )
      frequency = "mon" ;
    else if( mip_f == "day" )
      frequency = "day" ;
    else if( mip_f.substr(mip_f.size()-3) == "Day" )
      frequency = "day" ;
    else if( mip_f.substr(0,3) == "6hr" )
      frequency = "6hr" ;
    else if( mip_f.substr(0,3) == "3hr" )
      frequency = "3hr" ;
    else if( mip_f.substr(mip_f.size()-3) == "3hr" )
      frequency = "3hr" ;
  }

  return frequency ;
}

std::string
QA_Exp::getMIP_tableName(std::string tName)
{
  if( QA::tableId.size() )
    return QA::tableId;

  // the counter-parts in the attributes
  std::string tableID;
  if( tName.size() )
    tableID = tName ;
  else
    tableID = pQA->pIn->nc.getAttString(CMOR::n_table_id) ;

  Split x_tableID(tableID);

  if( x_tableID.size() > 1 )
  {
    std::string key("7_7a");

    if( notes->inq( key, CMOR::n_global) )
    {
      std::string capt( hdhC::tf_att(CMOR::n_global, CMOR::n_table_id, tableID) ) ;
      capt += " There should be no additional info included";

      (void) notes->operate(capt) ;
      pQA->setExitState( notes->getExitState() ) ;
    }

    return x_tableID[0];
  }

  if( x_tableID.size() > 1 )
  {
    std::string key("7_7b");

    if( notes->inq( key, CMOR::n_global) )
    {
      std::string capt( hdhC::tf_att(CMOR::n_global, CMOR::n_table_id, tableID) ) ;
      capt += " There should be no additional info included";

      (void) notes->operate(capt) ;
      pQA->setExitState( notes->getExitState() ) ;
    }
  }

  return x_tableID[0];
}

std::string
QA_Exp::getTableEntryID(std::string vName)
{
  vName += "," ;
  vName += getFrequency() ;

  return vName + ",";
}

std::string
QA_Exp::getVarnameFromFilename(std::string fName)
{
  if( ! fName.size() )
     fName = getVarnameFromFilename(pQA->pIn->file.getFilename()) ;

  size_t pos;
  if( (pos = fName.find("_")) < std::string::npos )
    fName = fName.substr(0,pos) ;

  return fName;
}

void
QA_Exp::init(std::vector<std::string>& optStr)
{
   // apply parsed command-line args
   applyOptions(optStr);

   if( pQA->isCheckDRS_F || pQA->isCheckCV || pQA->isCheckTime )
   {
     fVarname = getVarnameFromFilename(pQA->pIn->file.filename);
     getFrequency();
   }

   if( pQA->isCheckCV || pQA->isCheckData )
   {
     // Create and set VarMetaData objects.
     createVarMetaData() ;
   }

   // Check varname from filename with those in the file.
   // Is the shortname in the filename also defined in the nc-header?
   if( pQA->isCheckCV)
   {
     std::vector<std::string> vNames( pQA->pIn->nc.getVarName() ) ;
     size_t i;
     for( i=0 ; i < vNames.size() ; ++i )
       if( fVarname == vNames[i] )
         break;

     if( i == vNames.size() )
     {
       std::string key("1_3");
       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("variable ");
         capt += hdhC::tf_assign("acronym", fVarname);
         capt += " in the filename does not match any variable in the file" ;

         (void) notes->operate(capt) ;
         notes->setCheckStatus("CV",  pQA->n_fail );
       }
     }
   }

   return;
}

void
QA_Exp::initDataOutputBuffer(void)
{
  for( size_t i=0 ; i < varMeDa.size() ; ++i)
    varMeDa[i].qaData.initBuffer(pQA, pQA->currQARec, bufferSize);

  return;
}

void
QA_Exp::initDefaults(void)
{
  isUseStrict=false;
  bufferSize=1500;

  return;
}

void
QA_Exp::initResumeSession(std::vector<std::string>& prevTargets)
{
  if( !pQA->isCheckData )
    return;

  // a missing variable?
  for( size_t i=0 ; i < prevTargets.size() ; ++i)
  {
    size_t j;
    for( j=0 ; j < varMeDa.size() ; ++j)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( j == varMeDa.size() )
    {
       std::string key("3_14");
       if( notes->inq( key, prevTargets[i]) )
       {
         std::string capt("variable");
         capt += hdhC::tf_val(prevTargets[i]) ;
         capt += " is missing in sub-temporal file" ;

         if( notes->operate(capt) )
         notes->setCheckStatus("Consistency", pQA->n_fail );
         pQA->setExitState( notes->getExitState() ) ;
       }
    }
  }

  // a new variable?
  for( size_t j=0 ; j < varMeDa.size() ; ++j)
  {
    size_t i;
    for( i=0 ; i < prevTargets.size() ; ++i)
      if( prevTargets[i] == varMeDa[j].var->name )
        break;

    if( i == prevTargets.size() )
    {
       std::string key("3_15");
       if( notes->inq( key, pQA->fileStr) )
       {
         std::string capt("variable");
         capt += hdhC::tf_val(varMeDa[j].var->name);
         capt += " is new in sub-temporal file" ;

         if( notes->operate(capt) )
         notes->setCheckStatus("Consistency", pQA->n_fail );
         pQA->setExitState( notes->getExitState() ) ;
       }
    }
  }

  return;
}

bool
QA_Exp::inqTables(std::string tName)
{
  varReqTable.setFilename(tName + ".csv");

  if( ! varReqTable.exist() )
  {
     std::string key("7_12");

     if( notes->inq( key, pQA->fileStr) )
     {
        std::string capt("could not find ") ;
        capt += hdhC::tf_assign("table_id",varReqTable.getFile());

        (void) notes->operate(capt) ;
        notes->setCheckStatus("CMIP_Table", pQA->n_fail);
        pQA->setExitState( notes->getExitState() ) ;
     }

     return false;
  }

  return true;
}

/*
bool
QA_Exp::locate( GeoData<float> *gd, double *alat, double *alon, const char* crit )
{
  std::string str(crit);

// This is a TODO; but not needed for QA
  MtrxArr<float> &va=gd->getCellValue();
  MtrxArr<double> &wa=gd->getCellWeight();

  size_t i=0;
  double val;  // store extrem value
  int ei=-1;   // store index of extreme

  // find first valid value for initialisation
  for( ; i < va.size() ; ++i)
  {
      if( wa[i] == 0. )
        continue;

      val= va[i];
      ei=i;
      break;
  }

  // no value found
  if( ei < 0 )
  {
    *alat = 999.;
    *alon = 999.;
    return true;
  }

  if( str == "min" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] < val )
        {
          val= va[i];
          ei=i;
        }
    }
  }
  else if( str == "max" )
  {
    for( ; i < va.size() ; ++i)
    {
        if( wa[i] == 0. )
          continue;

        if( va[i] > val )
        {
          val= va[i];
          ei=i;
        }
    }
  }

  *alat = gd->getCellLatitude(static_cast<size_t>(ei) );
  *alon = gd->getCellLongitude(static_cast<size_t>(ei) );

  return false;
}
*/

void
QA_Exp::pushBackVarMeDa(Variable *var)
{
   varMeDa.push_back( VariableMetaData(pQA, var) );

   if( var )
   {
      VariableMetaData &vMD = varMeDa.back();

      vMD.forkAnnotation(notes);

      // disable tests by given options
      vMD.qaData.disableTests(var->name);

      vMD.qaData.init(pQA, var->name);

      // some more properties
      vMD.var->std_name = vMD.var->getAttValue(CMOR::n_standard_name);

      for( size_t i=0 ; i < var->attName.size() ; ++i )
      {
          std::string& an = var->attName[i] ;
          std::string& av = var->attValue[i][0];

          size_t j;
          // history, comment, etc.
          for( j=0 ; j < excludedAttribute.size() ; ++j )
              if( an == excludedAttribute[j] )
                 break;

          if( j < excludedAttribute.size() )
              continue;

          vMD.attMap[an] = hdhC::unique(av, hdhC::blank) ;

          if( an == CMOR::n_long_name )
          {
            if( notes->inq( "7_14", "", "INQ_ONLY") )
               vMD.attMap[an+"_insens"] = hdhC::Lower()(vMD.attMap[an]) ;
          }
      }

      // a special one
      vMD.attMap[CMOR::n_type] = pQA->pIn->nc.getVarTypeStr(var->name);
   }

   return;
}

void
QA_Exp::run(void)
{
  QA::tableId = getMIP_tableName() ;

  if( pQA->drs_cv_table.table_DRS_CV.is() )
  {
    if(pQA->isCheckDRS_F || pQA->isCheckDRS_P)
    {
      DRS_CV drsFN(pQA);
      drsFN.run();
    }
  }

  if( inqTables(QA::tableId) )
  {

    if(pQA->isCheckCV)
    {
      Variable* glob = &(pQA->pIn->variable[pQA->pIn->varSz]) ;

      // check CMIP6_CVs tables issues, i.e. meta-data in global attributes
      CMIP6_CV cmip6_cv(pQA, glob);
      cmip6_cv.run();

      // get meta data from file and compare with tables
      checkMetaData(*(pQA->pIn));
    }
  }

  return ;
}

void
QA_Exp::setParent(QA* p)
{
   pQA = p;
   notes = p->notes;
   return;
}

VariableMetaData::VariableMetaData(QA *p, Variable *v)
{
   pQA = p;

   if( v )
   {
     var = v;
     isNewVar=false;
   }
   else
   {
     var = new Variable ;
     isNewVar=true;
   }
}

VariableMetaData::~VariableMetaData()
{
  if( isNewVar )
    delete var;

  dataOutputBuffer.clear();
}

void
VariableMetaData::verifyPercent(void)
{
   // % range
   if( var->units == "%" )
   {
      if( qaData.statMin.getSampleMin() >= 0.
           && qaData.statMax.getSampleMax() <= 1. )
      {
        std::string key("6_8");
        if( notes->inq( key, var->name) )
        {
          std::string capt( hdhC::tf_var(var->name, ":"));
          capt += "Suspicion of fractional data range for units [%], found range ";
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV",  pQA->n_fail );
        }
      }
   }

   if( var->units == "1" || var->units.size() == 0 )
   {
      // is it % range? Not all cases are detectable
      if( qaData.statMin.getSampleMin() >= 0. &&
           qaData.statMax.getSampleMax() > 1.
             && qaData.statMax.getSampleMax() <= 100.)
      {
        std::string key("6_9");
        if( notes->inq( key, var->name) )
        {
          std::string capt( "Suspicion of percentage data range for units <1>, found range " ) ;
          capt += "[" + hdhC::double2String(qaData.statMin.getSampleMin());
          capt += ", " + hdhC::double2String(qaData.statMax.getSampleMax()) + "]" ;

          (void) notes->operate(capt) ;
          notes->setCheckStatus("CV",  pQA->n_fail );
        }
      }
   }

   return;
}

int
VariableMetaData::finally(int xCode)
{
  // write pending results to qa-file.nc. Modes are considered there
  qaData.flush();

  // annotation obj forked by the parent VMD
  notes->printFlags();

  int rV = notes->getExitState();
  xCode = ( xCode > rV ) ? xCode : rV ;

  return xCode ;
}

void
VariableMetaData::forkAnnotation(Annotation *n)
{
   notes = new Annotation(n);

   // this is not a mistaken
   qaData.setAnnotation(n);

   return;
}

void
VariableMetaData::setAnnotation(Annotation *n)
{
   notes = n;

   qaData.setAnnotation(n);

   return;
}