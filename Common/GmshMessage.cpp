// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "GmshConfig.h"
#include "GmshMessage.h"
#include "Gmsh.h"
#include "Options.h"
#include "Context.h"
#include "OS.h"

#if defined(HAVE_MPI)
#include <mpi.h>
#endif

#if defined(HAVE_FLTK)
#include <FL/fl_ask.H>
#include "GUI.h"
#include "messageWindow.h"
#include "extraDialogs.h"
#endif

int Msg::_commRank = 0;
int Msg::_commSize = 1;
int Msg::_verbosity = 4;
int Msg::_progressMeterStep = 10;
int Msg::_progressMeterCurrent = 0;
std::map<std::string, double> Msg::_timers;
int Msg::_warningCount = 0;
int Msg::_errorCount = 0;
GmshMessage *Msg::_callback = 0;
std::string Msg::_commandLine;
std::string Msg::_launchDate;

#if defined(HAVE_NO_VSNPRINTF)
static int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
  if(strlen(fmt) > size - 1){ // just copy the format
    strncpy(str, fmt, size - 1);
    str[size - 1] = '\0';
    return size;
  }
  return vsprintf(str, fmt, ap);
}
#endif

#if defined(_MSC_VER) && (_MSC_VER == 1310) //NET 2003
#define vsnprintf _vsnprintf
#endif

void Msg::Init(int argc, char **argv)
{
#if defined(HAVE_MPI)
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &_commRank);
  MPI_Comm_size(MPI_COMM_WORLD, &_commSize);
  MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
#endif
  time_t now;
  time(&now);
  _launchDate = ctime(&now);
  _launchDate.resize(_launchDate.size() - 1);
  _commandLine.clear();
  for(int i = 0; i < argc; i++){
    if(i) _commandLine += " ";
    _commandLine += argv[i];
  }
}

void Msg::Exit(int level)
{
  // delete the temp file
  if(!_commRank) UnlinkFile(CTX::instance()->homeDir + CTX::instance()->tmpFileName);

  // exit directly on abnormal program termination (level != 0). We
  // used to call abort() to flush open streams, but on modern OSes
  // this calls the annoying "report this crash to the mothership"
  // window... so just exit!
  if(level){
#if defined(HAVE_MPI)
    MPI_Finalize();
#endif
    exit(level);
  }
  
#if defined(HAVE_FLTK)
  // if we exit cleanly (level==0) and we are in full GUI mode, save
  // the persistent info to disk
  if(GUI::available() && !_commRank) {
    if(CTX::instance()->sessionSave)
      Print_Options(0, GMSH_SESSIONRC, 0, 0, 
                    (CTX::instance()->homeDir + CTX::instance()->sessionFileName).c_str());
    if(CTX::instance()->optionsSave)
      Print_Options(0, GMSH_OPTIONSRC, 1, 0, 
                    (CTX::instance()->homeDir + CTX::instance()->optionsFileName).c_str());
  }
#endif

#if defined(HAVE_MPI)
  MPI_Finalize();
#endif
  exit(0);
}

void Msg::Fatal(const char *fmt, ...)
{
  _errorCount++;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Fatal", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    GUI::instance()->check();
    std::string tmp = std::string("@C1@.") + "Fatal   : " + str;
    GUI::instance()->messages->add(tmp.c_str());
    GUI::instance()->messages->show();
    GUI::instance()->messages->save
      ((CTX::instance()->homeDir + CTX::instance()->errorFileName).c_str());
    fl_alert("A fatal error has occurred which will force Gmsh to abort.\n"
             "The error messages have been saved in the following file:\n\n%s",
             (CTX::instance()->homeDir + CTX::instance()->errorFileName).c_str());
  }
#endif

  if(CTX::instance()->terminal){
    if(_commSize > 1)
      fprintf(stderr, "Fatal   : [On processor %d] %s\n", _commRank, str);
    else
      fprintf(stderr, "Fatal   : %s\n", str);
    fflush(stderr);
  }

  // only exit if a callback is not provided
  if(!_callback) Exit(1);
}

void Msg::Error(const char *fmt, ...)
{
  _errorCount++;

  if(_verbosity < 1) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Error", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    GUI::instance()->check();
    std::string tmp = std::string("@C1@.") + "Error   : " + str;
    GUI::instance()->messages->add(tmp.c_str());
    GUI::instance()->messages->show();
  }
#endif

  if(CTX::instance()->terminal){
    if(_commSize > 1) 
      fprintf(stderr, "Error   : [On processor %d] %s\n", _commRank, str);
    else
      fprintf(stderr, "Error   : %s\n", str);
    fflush(stderr);
  }
}

void Msg::Warning(const char *fmt, ...)
{
  _warningCount++;

  if(_commRank || _verbosity < 2) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Warning", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    GUI::instance()->check();
    std::string tmp = std::string("@C1@.") + "Warning : " + str;
    GUI::instance()->messages->add(tmp.c_str());
  }
#endif

  if(CTX::instance()->terminal){
    fprintf(stderr, "Warning : %s\n", str);
    fflush(stderr);
  }
}

void Msg::Info(const char *fmt, ...)
{
  if(_commRank || _verbosity < 3) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Info", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    GUI::instance()->check();
    std::string tmp = std::string("Info    : ") + str;
    GUI::instance()->messages->add(tmp.c_str());
  }
#endif

  if(CTX::instance()->terminal){
    fprintf(stdout, "Info    : %s\n", str);
    fflush(stdout);
  }
}

void Msg::Direct(const char *fmt, ...)
{
  if(_commRank || _verbosity < 3) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  Direct(3, str);
}

void Msg::Direct(int level, const char *fmt, ...)
{
  if(_commRank || _verbosity < level) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Direct", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    GUI::instance()->check();
    std::string tmp;
    if(level < 3)
      tmp = std::string("@C1@.") + str;
    else
      tmp = std::string("@C4@.") + str;
    GUI::instance()->messages->add(tmp.c_str());
    if(level == 1)
      GUI::instance()->messages->show();
  }
#endif
  
  if(CTX::instance()->terminal){
    fprintf(stdout, "%s\n", str);
    fflush(stdout);
  }
}

void Msg::StatusBar(int num, bool log, const char *fmt, ...)
{
  if(_commRank || _verbosity < 4) return;
  if(num < 1 || num > 3) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback && log) (*_callback)("Info", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    if(log) GUI::instance()->check();
    GUI::instance()->setStatus(str, num - 1);
    if(log){
      std::string tmp = std::string("Info    : ") + str;
      GUI::instance()->messages->add(tmp.c_str());
    }
  }
#endif

  if(log && CTX::instance()->terminal){
    fprintf(stdout, "Info    : %s\n", str);
    fflush(stdout);
  }
}

void Msg::Debug(const char *fmt, ...)
{
  if(_verbosity < 99) return;

  char str[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  if(_callback) (*_callback)("Debug", str);

#if defined(HAVE_FLTK)
  if(GUI::available()){
    std::string tmp = std::string("Debug   : ") + str;
    GUI::instance()->messages->add(tmp.c_str());
  }
#endif

  if(CTX::instance()->terminal){
    if(_commSize > 1) 
      fprintf(stdout, "Debug   : [On processor %d] %s\n", _commRank, str);
    else
      fprintf(stdout, "Debug   : %s\n", str);
    fflush(stdout);
  }
}

void Msg::ProgressMeter(int n, int N, const char *fmt, ...)
{
  if(_commRank || _verbosity < 3) return;

  double percent = 100. * (double)n/(double)N;

  if(percent >= _progressMeterCurrent){
    char str[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);

    if(strlen(fmt)) strcat(str, " ");

    char str2[1024];
    sprintf(str2, "(%d %%)", _progressMeterCurrent);
    strcat(str, str2);

#if defined(HAVE_FLTK)
    if(GUI::available()){
      GUI::instance()->setStatus(str, 1);
      GUI::instance()->check();
    }
#endif
    if(CTX::instance()->terminal){
      fprintf(stdout, "%s                     \r", str);
      fflush(stdout);
    }

    while(_progressMeterCurrent < percent)
      _progressMeterCurrent += _progressMeterStep;
  }

  if(n > N - 1){
#if defined(HAVE_FLTK)
    if(GUI::available()) GUI::instance()->setStatus("", 1);
#endif
    if(CTX::instance()->terminal){
      fprintf(stdout, "Done!                                              \r");
      fflush(stdout);
    }
  }
}

void Msg::PrintTimers()
{
  // do a single stdio call!
  std::string str;
  for(std::map<std::string, double>::iterator it = _timers.begin(); 
      it != _timers.end(); it++){
    if(it != _timers.begin()) str += ", ";
    char tmp[256];
    sprintf(tmp, "%s = %gs ", it->first.c_str(), it->second);
    str += std::string(tmp);
  }
  if(!str.size()) return;

  if(CTX::instance()->terminal){
    if(_commSize > 1) 
      fprintf(stdout, "Timers  : [On processor %d] %s\n", _commRank, str.c_str());
    else
      fprintf(stdout, "Timers  : %s\n", str.c_str());
    fflush(stdout);
  }
}

void Msg::PrintErrorCounter(const char *title)
{
  if(_commRank || _verbosity < 1) return;
  if(!_warningCount && !_errorCount) return;

  std::string prefix = _errorCount ? "Error   : " : "Warning : ";
  std::string help("Check the full log for details");
  std::string line(std::max(strlen(title), help.size()), '-');
  char warn[128], err[128];
  sprintf(warn, "%5d warning%s", _warningCount, _warningCount == 1 ? "" : "s");
  sprintf(err, "%5d error%s", _errorCount, _errorCount == 1 ? "" : "s");

#if defined(HAVE_FLTK)
  if(GUI::available()){
    std::string red("@C1@.");
    GUI::instance()->messages->add((red + prefix + line).c_str());
    GUI::instance()->messages->add((red + prefix + title).c_str());
    GUI::instance()->messages->add((red + prefix + warn).c_str());
    GUI::instance()->messages->add((red + prefix + err).c_str());
    GUI::instance()->messages->add((red + prefix + help).c_str());
    GUI::instance()->messages->add((red + prefix + line).c_str());
    if(_errorCount){
      GUI::instance()->messages->show();
      fl_beep();
    }
  }
#endif

  if(CTX::instance()->terminal){
    fprintf(stderr, "%s\n%s\n%s\n%s\n%s\n%s\n", (prefix + line).c_str(), 
	    (prefix + title).c_str(), (prefix + warn).c_str(),
	    (prefix + err).c_str(), (prefix + help).c_str(), 
	    (prefix + line).c_str());
    fflush(stderr);
  }
}

double Msg::GetValue(const char *text, double defaultval)
{
  // if a callback is given let's assume we don't want to be bothered
  // with interactive stuff
  if(CTX::instance()->noPopup || _callback) return defaultval;

#if defined(HAVE_FLTK)
  if(GUI::available()){
    char defaultstr[256];
    sprintf(defaultstr, "%.16g", defaultval);
    const char *ret = fl_input(text, defaultstr);
    if(!ret)
      return defaultval;
    else
      return atof(ret);
  }
#endif

  printf("%s (default=%.16g) ", text, defaultval);
  char str[256];
  char *ret = fgets(str, sizeof(str), stdin);
  if(!ret || !strlen(str) || !strcmp(str, "\n"))
    return defaultval;
  else
    return atof(str);
}

bool Msg::GetBinaryAnswer(const char *question, const char *yes, 
			      const char *no, bool defaultval)
{
  // if a callback is given let's assume we don't want to be bothered
  // with interactive stuff
  if(CTX::instance()->noPopup || _callback) return defaultval;

#if defined(HAVE_FLTK)
  if(GUI::available()){
    if(fl_choice(question, no, yes, NULL))
      return true;
    else
      return false;
  }
#endif

  while(1){
    printf("%s\n\n[%s] or [%s]? (default=%s) ", question, yes, no, 
	   defaultval ? yes : no);
    char str[256];
    char *ret = fgets(str, sizeof(str), stdin);
    if(!ret || !strlen(str) || !strcmp(str, "\n"))
      return defaultval;
    else if(!strcmp(str, yes))
      return true;
    else if(!strcmp(str, no))
      return false;
  }
}

void Msg::Barrier()
{
#if defined(HAVE_MPI)
  MPI_Barrier(MPI_COMM_WORLD);
#endif
}

#if defined(_OPENMP)

#include <omp.h>

int Msg::GetNumThreads(){ return omp_get_num_threads(); }
int Msg::GetMaxThreads(){ return omp_get_max_threads(); }
int Msg::GetThreadNum(){ return omp_get_thread_num(); }

#else

int Msg::GetNumThreads(){ return 1; }
int Msg::GetMaxThreads(){ return 1; }
int Msg::GetThreadNum(){ return 0; }

#endif
