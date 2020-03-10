
#ifndef ZIX_PROCEDURE_STEP_HANLDER_H
#define ZIX_PROCEDURE_STEP_HANLDER_H

#include <list>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_result.h"

#ifdef GLOBAL_INSTALLATION

#define ZIX_PROC_STEP_FILE "/opt/transfer/libzix_proc_step.xml"
#define ZIX_PROC_STEP_FILE_TMP "/opt/transfer/libzix_proc_step.xml.tmp"

#define ZIX_PROC_SAVE_FILE "/opt/transfer/libzix_proc_save.xml"
#define ZIX_PROC_SAVE_FILE_TMP "/opt/transfer/libzix_proc_save.xml.tmp"

#define ZIX_EXEC_ATTR_FILE "/opt/transfer/libzix_id_exec.xml"

#define ZIX_PROC_RESULT_FILE "/opt/transfer/libzix_proc_result.xml"
#define ZIX_UPDATE_RESULT_FILE "/opt/transfer/libzix_update_result.xml"
#define ZIX_UPDATE_SAVE_FILE "/opt/transfer/libzix_update_save.xml"

#define ZIX_PROC_STEP_MAX_RETRIES 3

#else

#define ZIX_PROC_STEP_FILE "/tmp/libzix_proc_step.xml"
#define ZIX_PROC_STEP_FILE_TMP "/tmp/libzix_proc_step.xml.tmp"

#define ZIX_PROC_SAVE_FILE "/tmp/libzix_proc_save.xml"
#define ZIX_PROC_SAVE_FILE_TMP "/tmp/libzix_proc_save.xml.tmp"

#define ZIX_EXEC_ATTR_FILE "/tmp/libzix_id_exec.xml"

#define ZIX_PROC_RESULT_FILE "/tmp/libzix_proc_result.xml"
#define ZIX_UPDATE_RESULT_FILE "/tmp/libzix_update_result.xml"
#define ZIX_UPDATE_SAVE_FILE "/tmp/libzix_update_save.xml"

#define ZIX_PROC_STEP_MAX_RETRIES 3

#endif

bool check_exec_exceeded(const Glib::ustring& id, const int exec);
void create_procedure_exec_file(const Glib::ustring& id);
bool increment_exec_attribute(const Glib::ustring& id);
int get_current_proc_step ();
int get_current_retry ();
const Glib::ustring & get_current_interface ();
const Glib::ustring & get_current_result_filename ();
const std::list <Glib::RefPtr <XmlResult> > & get_current_result_list ();
bool procedure_in_progress();

void note_current_proc_step (int step, int retry = 0);
void note_current_proc_step (int step, const Glib::ustring &interface, const Glib::ustring & result_filename);

void note_current_proc_step (int step, const std::list <Glib::RefPtr <XmlResult> > & result_list);
void note_current_proc_step (int step, int retry, const std::list <Glib::RefPtr <XmlResult> > & result_list);
#endif
