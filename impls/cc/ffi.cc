#include "ffi.hh"
#include <dlfcn.h>
#include <ffi.h>
#include <map>

namespace
{
    std::map<std::string, ffi_type *> types = {
        {"void", &ffi_type_void},
        {"char", &ffi_type_schar},
        {"signed char", &ffi_type_schar},
        {"unsigned char", &ffi_type_uchar},
        {"short", &ffi_type_sshort},
        {"signed short", &ffi_type_sshort},
        {"unsigned short", &ffi_type_ushort},
        {"int", &ffi_type_sint},
        {"signed int", &ffi_type_sint},
        {"unsigned int", &ffi_type_uint},
        {"long", &ffi_type_slong},
        {"signed long", &ffi_type_slong},
        {"unsigned long", &ffi_type_ulong},
        {"float", &ffi_type_float},
        {"double", &ffi_type_double},
        {"int8_t", &ffi_type_sint8},
        {"uint8_t", &ffi_type_uint8},
        {"int16_t", &ffi_type_sint16},
        {"uint16_t", &ffi_type_uint16},
        {"int32_t", &ffi_type_sint32},
        {"uint32_t", &ffi_type_uint32},
        {"int64_t", &ffi_type_sint64},
        {"uint64_t", &ffi_type_uint64},
        {"size_t", &ffi_type_uint64},
    };
}

std::shared_ptr<MalType> c_eval(std::vector<std::shared_ptr<MalType>> args)
{
    std::string libname = static_cast<MalSymbol &>(*args[0]);
    std::string retname = static_cast<MalSymbol &>(*args[1]);
    std::string funcname = static_cast<MalSymbol &>(*args[2]);

    void *dl_handle;
    if (libname == "nil")
        dl_handle = dlopen(NULL, RTLD_LAZY);
    else
        dl_handle = dlopen(libname.c_str(), RTLD_LAZY);

    void *func = dlsym(dl_handle, funcname.c_str());
    char *msg = dlerror();
    if (msg)
    {
        dlclose(dl_handle);
        throw std::runtime_error(msg);
    }

    dlclose(dl_handle);

    ffi_type *rettype;
    if (*retname.rbegin() == '*')
        rettype = &ffi_type_pointer;
    else
        rettype = types.at(retname);

    std::vector<ffi_type *> argtypes;
    std::vector<void *> argvalues;
    std::vector<int> intargs;
    std::vector<std::string> strargs;

    for (unsigned i = 3; i < args.size(); i += 2)
    {
        std::string argtype = static_cast<MalSymbol &>(*args[i]);
        if (*argtype.rbegin() == '*')
            argtypes.push_back(&ffi_type_pointer);
        else
            argtypes.push_back(types.at(argtype));

        auto arg = args[i + 1];
        if (arg->type() == MalType::Type::Int)
        {
            intargs.push_back(static_cast<MalInt &>(*arg));
            argvalues.push_back(&(*intargs.rbegin()));
        }
        else if (arg->type() == MalType::Type::Symbol)
        {
            strargs.push_back(static_cast<MalSymbol &>(*arg));
            argvalues.push_back(const_cast<char *>(strargs.rbegin()->c_str()));
        }
        else
            throw std::invalid_argument("c_eval");
    }

    ffi_cif cif;
    if (FFI_OK == ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argtypes.size(), rettype, argtypes.data()))
    {
        size_t retval = 0;
        ffi_call(&cif, FFI_FN(func), &retval, argvalues.data());
        if (*retname.rbegin() == '*')
            return std::make_shared<MalSymbol>((char *)retval);
        else
            return std::make_shared<MalInt>(retval);
    }

    throw std::runtime_error("c_eval");
}
