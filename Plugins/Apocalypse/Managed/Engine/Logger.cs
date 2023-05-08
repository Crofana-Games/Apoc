


using System.Runtime.InteropServices;

namespace Engine;

public unsafe static class Logger
{
    public static void Verbose(string message) => Log(LogVerbosity.Verbose, message);
    public static void Log(string message) => Log(LogVerbosity.Log, message);
    public static void Warning(string message) => Log(LogVerbosity.Warning, message);
    public static void Error(string message) => Log(LogVerbosity.Error, message);
    public static void Fatal(string message) => Log(LogVerbosity.Fatal, message);

    internal enum LogVerbosity
    {
        Verbose,
        Log,
        Warning,
        Error,
        Fatal,
    }

    internal static delegate* unmanaged<LogVerbosity, IntPtr, void> UnmanagedLog;

    private static void Log(LogVerbosity logVerbosity, string message)
    {
        IntPtr messagePtr = Marshal.StringToHGlobalUni(message);
        UnmanagedLog(logVerbosity, messagePtr);
        Marshal.FreeHGlobal(messagePtr);
    }

}