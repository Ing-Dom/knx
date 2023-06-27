#pragma once

enum LogLevelKnx
{
    LogLevelKnx_None,
    LogLevelKnx_Fatal,
    LogLevelKnx_Error,
    LogLevelKnx_Info,
    LogLevelKnx_Debug,
    LogLevelKnx_Trace
};

class LogKnx
{
public:
  LogKnx();
  void logMessage(LogLevelKnx logLevel);
};