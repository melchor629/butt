//
//  DynamicLibraryLoad.hpp
//  butt
//
//  Created by Melchor Garau Madrigal on 17/2/16.
//  Copyright © 2016 Daniel Nöthen. All rights reserved.
//

#ifndef DynamicLibraryLoad_h
#define DynamicLibraryLoad_h

#ifdef WIN32
#include <windows.h>
#define LIBRARY_EXTENSION "dll"
#else
#include <dlfcn.h>
#if defined(__APPLE__) && defined(__MACH__)
#define LIBRARY_EXTENSION "dylib"
#else
#define LIBRARY_EXTENSION "so"
#endif
#endif

#include <exception>
#include <string>

class DynamicLibraryException : public std::exception {
private:
    std::string msg;

public:
    DynamicLibraryException(std::string what) throw() {
        msg = what;
    }

    ~DynamicLibraryException() throw() {}

    virtual const char* what() const throw() {
        return msg.c_str();
    }
};

class DynamicLibrary {
private:
#ifdef WIN32
    HMODULE lib = NULL;
#else
    void* lib = NULL;
#endif

    void delegatedConstructor(const char* libname, const char* ext) throw(DynamicLibraryException) {
        char str[100];
        snprintf(str, 100, "%s.%s", libname, ext);
#ifdef WIN32
        lib = LoadLibrary(str);
        if(lib == NULL) {
            //See https://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
            snprintf(str, 100, "Could not load %s: %d", libname, GetLastError());
            throw DynamicLibraryException(std::string(str));
        }
#else
        dlerror();
        lib = dlopen(str, RTLD_LAZY);
        if(lib == NULL) {
            snprintf(str, 100, "Could not load %s: %s", libname, dlerror());
            throw DynamicLibraryException(std::string(str));
        }
#endif
    }

public:
    DynamicLibrary(const char* libname) throw(DynamicLibraryException) {
        delegatedConstructor(libname, LIBRARY_EXTENSION);
    }

    DynamicLibrary(const char* libname, const char* ext) throw(DynamicLibraryException) {
        delegatedConstructor(libname, ext);
    }

    ~DynamicLibrary() {
        if(lib != NULL) {
#ifdef WIN32
            FreeLibrary(lib);
#else
            dlclose(lib);
#endif
            lib = NULL;
        }
    }

    template <typename T>
    T getFunctionPointer(const char* function) throw(DynamicLibraryException) {
        char str[50];
        void* func;
#ifdef WIN32
        func = GetProcAddress(lib, function);
        if(func == NULL) {
            snprintf(str, 50, "Could not load function \"%s\": %d", function, GetLastError());
            throw DynamicLibraryException(std::string(str));
        }
#else
        func = dlsym(lib, function);
        if(func == NULL) {
            snprintf(str, 50, "Could not load function \"%s\": %s", function, dlerror());
            throw DynamicLibraryException(std::string(str));
        }
#endif

        union {void* ptr; void (*func)(); } d;
        d.ptr = func;
        return (T) d.func;
    }

};

#endif /* DynamicLibraryLoad_h */
