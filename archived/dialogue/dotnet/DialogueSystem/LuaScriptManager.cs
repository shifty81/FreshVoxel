using System;
using System.Collections.Generic;
using NLua;

namespace FreshEngine.DialogueSystem;

/// <summary>
/// Manages Lua script execution for dialogue system
/// </summary>
public class LuaScriptManager : IDisposable
{
    private Lua? _luaState;
    private bool _disposed;

    /// <summary>
    /// Initializes the Lua scripting environment
    /// </summary>
    public LuaScriptManager()
    {
        try
        {
            _luaState = new Lua();
            _luaState.State.Encoding = System.Text.Encoding.UTF8;
            RegisterDefaultFunctions();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Warning: Lua initialization failed: {ex.Message}");
            Console.WriteLine("Dialogue system will work without Lua scripting support.");
        }
    }

    /// <summary>
    /// Checks if Lua is available
    /// </summary>
    public bool IsLuaAvailable => _luaState != null;

    /// <summary>
    /// Registers default functions available to Lua scripts
    /// </summary>
    private void RegisterDefaultFunctions()
    {
        if (_luaState == null) return;

        // Register log function
        _luaState.RegisterFunction("log", this, typeof(LuaScriptManager).GetMethod(nameof(Log)));
        
        // Register print function (alias for log)
        _luaState.RegisterFunction("print", this, typeof(LuaScriptManager).GetMethod(nameof(Log)));
    }

    /// <summary>
    /// Log function accessible from Lua
    /// </summary>
    public void Log(string message)
    {
        Console.WriteLine($"[Lua] {message}");
    }

    /// <summary>
    /// Sets a global variable in the Lua environment
    /// </summary>
    public void SetGlobal(string name, object value)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot set global variable");
            return;
        }

        try
        {
            _luaState[name] = value;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error setting Lua global '{name}': {ex.Message}");
        }
    }

    /// <summary>
    /// Gets a global variable from the Lua environment
    /// </summary>
    public object? GetGlobal(string name)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot get global variable");
            return null;
        }

        try
        {
            return _luaState[name];
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error getting Lua global '{name}': {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Executes a Lua script string
    /// </summary>
    public bool ExecuteScript(string script)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot execute script");
            return false;
        }

        if (string.IsNullOrWhiteSpace(script))
            return true;

        try
        {
            _luaState.DoString(script);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Lua script execution error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Executes a Lua script from a file
    /// </summary>
    public bool ExecuteScriptFile(string filePath)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot execute script file");
            return false;
        }

        if (!System.IO.File.Exists(filePath))
        {
            Console.WriteLine($"Lua script file not found: {filePath}");
            return false;
        }

        try
        {
            _luaState.DoFile(filePath);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Lua script file execution error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Calls a Lua function
    /// </summary>
    public object? CallFunction(string functionName, params object[] args)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot call function");
            return null;
        }

        try
        {
            var function = _luaState.GetFunction(functionName);
            if (function == null)
            {
                Console.WriteLine($"Lua function '{functionName}' not found");
                return null;
            }

            return function.Call(args);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error calling Lua function '{functionName}': {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Evaluates a Lua condition and returns the result
    /// </summary>
    public bool EvaluateCondition(string condition)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, defaulting condition to false");
            return false;
        }

        if (string.IsNullOrWhiteSpace(condition))
            return false;

        try
        {
            // Wrap in return statement if not present
            var script = condition.Trim().StartsWith("return") 
                ? condition 
                : $"return {condition}";

            var result = _luaState.DoString(script);
            
            if (result != null && result.Length > 0)
            {
                var value = result[0];
                if (value is bool boolValue)
                    return boolValue;
                if (value is double doubleValue)
                    return doubleValue != 0;
                if (value is string strValue)
                    return !string.IsNullOrEmpty(strValue);
            }

            return false;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error evaluating Lua condition: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Registers a C# function to be callable from Lua
    /// </summary>
    public void RegisterFunction(string name, object target, string methodName)
    {
        if (_luaState == null)
        {
            Console.WriteLine("Warning: Lua not available, cannot register function");
            return;
        }

        try
        {
            var method = target.GetType().GetMethod(methodName);
            if (method != null)
            {
                _luaState.RegisterFunction(name, target, method);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error registering Lua function '{name}': {ex.Message}");
        }
    }

    /// <summary>
    /// Clears all Lua state
    /// </summary>
    public void Reset()
    {
        Dispose();
        _luaState = new Lua();
        _luaState.State.Encoding = System.Text.Encoding.UTF8;
        RegisterDefaultFunctions();
    }

    /// <summary>
    /// Disposes the Lua state
    /// </summary>
    public void Dispose()
    {
        if (!_disposed)
        {
            _luaState?.Dispose();
            _luaState = null;
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
