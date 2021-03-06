#ifndef MANTID_PYTHONINTERFACE_KERNEL_GETPOINTER_H_
#define MANTID_PYTHONINTERFACE_KERNEL_GETPOINTER_H_

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER > 190023918
// Visual Studio Update 3 refuses to link boost python exports that use
// register_ptr_to_python with a virtual base. This is a work around
#define GET_POINTER_SPECIALIZATION(TYPE)                                       \
  namespace boost {                                                            \
  template <> const volatile TYPE *get_pointer(const volatile TYPE *p) {       \
    return p;                                                                  \
  }                                                                            \
  }
#else
#define GET_POINTER_SPECIALIZATION(TYPE)
#endif
#endif // MANTID_PYTHONINTERFACE_KERNEL_GETPOINTER_H_