#include <sys/ioccom.h>

#define RT_OS_DARWIN
#define RT_ARCH_X86

#define SUP_DARWIN_IOSERVICE_COOKIE            0x64726962 /* 'bird' */

#define RT_SIZEOFMEMB(type, member)             ( sizeof(((type *)(void *)0)->member) )

#define VERR_INTERNAL_ERROR                 (-225)

#define CTL_CODE(DeviceType, Function, Method_ignored, Access_ignored) \
    ( (3 << 30) | ((DeviceType) << 8) | (Function) | (sizeof(SUPDRVIOCTLDATA) << 16) )

#define FILE_DEVICE_UNKNOWN    0x00000022
#define SUP_IOCTL_FLAG     128

#define SUP_IOCTL_LDR_LOAD                              SUP_CTL_CODE_BIG(4)
#define SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage)             RT_UOFFSETOF(SUPLDRLOAD, u.In.abImage[cbImage])


#define SUP_CTL_CODE_SIZE(Function, Size)      _IOC(IOC_INOUT, 'V', (Function) | SUP_IOCTL_FLAG, (Size))
#define SUP_CTL_CODE_BIG(Function)             _IO('V', (Function) | SUP_IOCTL_FLAG)
#define SUP_CTL_CODE_FAST(Function)            _IO('V', (Function) | SUP_IOCTL_FLAG)
#define SUP_CTL_CODE_NO_SIZE(uIOCtl)           ( (uIOCtl) & ~_IOC(0,0,0,IOCPARM_MASK) )

#define RT_UOFFSETOF(type, member)             ( (uintptr_t)&( ((type *)(void *)0)->member) )


/**
 * Common In/Out header.
 */
typedef struct SUPREQHDR
{
    /** Cookie. */
    uint32_t        u32Cookie;
    /** Session cookie. */
    uint32_t        u32SessionCookie;
    /** The size of the input. */
    uint32_t        cbIn;
    /** The size of the output. */
    uint32_t        cbOut;
    /** Flags. See SUPREQHDR_FLAGS_* for details and values. */
    uint32_t        fFlags;
    /** The VBox status code of the operation, out direction only. */
    int32_t         rc;
} SUPREQHDR;
/** Pointer to a IOC header. */
typedef SUPREQHDR *PSUPREQHDR;

/** @name SUPREQHDR::fFlags values
 * @{  */
/** Masks out the magic value.  */
#define SUPREQHDR_FLAGS_MAGIC_MASK                      UINT32_C(0xff0000ff)
/** The generic mask. */
#define SUPREQHDR_FLAGS_GEN_MASK                        UINT32_C(0x0000ff00)
/** The request specific mask. */
#define SUPREQHDR_FLAGS_REQ_MASK                        UINT32_C(0x00ff0000)

/** There is extra input that needs copying on some platforms. */
#define SUPREQHDR_FLAGS_EXTRA_IN                        UINT32_C(0x00000100)
/** There is extra output that needs copying on some platforms. */
#define SUPREQHDR_FLAGS_EXTRA_OUT                       UINT32_C(0x00000200)

/** The magic value. */
#define SUPREQHDR_FLAGS_MAGIC                           UINT32_C(0x42000042)
/** The default value. Use this when no special stuff is requested. */
#define SUPREQHDR_FLAGS_DEFAULT                         SUPREQHDR_FLAGS_MAGIC
/** @} */


/** @name SUP_IOCTL_COOKIE
 * @{
 */
/** Negotiate cookie. */
#define SUP_IOCTL_COOKIE                                SUP_CTL_CODE_SIZE(1, SUP_IOCTL_COOKIE_SIZE)
/** The request size. */
#define SUP_IOCTL_COOKIE_SIZE                           sizeof(SUPCOOKIE)
/** The SUPREQHDR::cbIn value. */
#define SUP_IOCTL_COOKIE_SIZE_IN                        sizeof(SUPREQHDR) + RT_SIZEOFMEMB(SUPCOOKIE, u.In)
/** The SUPREQHDR::cbOut value. */
#define SUP_IOCTL_COOKIE_SIZE_OUT                       sizeof(SUPREQHDR) + RT_SIZEOFMEMB(SUPCOOKIE, u.Out)
/** SUPCOOKIE_IN magic word. */
#define SUPCOOKIE_MAGIC                                 "The Magic Word!"
/** The initial cookie. */
#define SUPCOOKIE_INITIAL_COOKIE                        0x69726f74 /* 'tori' */

/** Current interface version.
 * The upper 16-bit is the major version, the lower the minor version.
 * When incompatible changes are made, the upper major number has to be changed.
 *
 * Update rules:
 *  -# Only update the major number when incompatible changes have been made to
 *     the IOC interface or the ABI provided via the functions returned by
 *     SUPQUERYFUNCS.
 *  -# When adding new features (new IOC number, new flags, new exports, ++)
 *     only update the minor number and change SUPLib.cpp to require the
 *     new IOC version.
 *  -# When incrementing the major number, clear the minor part and reset
 *     any IOC version requirements in SUPLib.cpp.
 *  -# When increment the major number, execute all pending work.
 *
 * @todo Pending work on next major version change:
 *          - nothing.
 *
 * @remarks 0x002a0000 is used by 5.1. The next version number must be 0x002b0000.
 */
#define SUPDRV_IOC_VERSION                              0x00290001

/** SUP_IOCTL_COOKIE. */
typedef struct SUPCOOKIE
{
    /** The header.
     * u32Cookie must be set to SUPCOOKIE_INITIAL_COOKIE.
     * u32SessionCookie should be set to some random value. */
    SUPREQHDR               Hdr;
    union
    {
        struct
        {
            /** Magic word. */
            char            szMagic[16];
            /** The requested interface version number. */
            uint32_t        u32ReqVersion;
            /** The minimum interface version number. */
            uint32_t        u32MinVersion;
        } In;
        struct
        {
            /** Cookie. */
            uint32_t        u32Cookie;
            /** Session cookie. */
            uint32_t        u32SessionCookie;
            /** Interface version for this session. */
            uint32_t        u32SessionVersion;
            /** The actual interface version in the driver. */
            uint32_t        u32DriverVersion;
            /** Number of functions available for the SUP_IOCTL_QUERY_FUNCS request. */
            uint32_t        cFunctions;
            /** Session handle. */
            //R0PTRTYPE(PSUPDRVSESSION)   pSession;
            uint64_t        pSession;
        } Out;
    } u;
} SUPCOOKIE, *PSUPCOOKIE;
/** @} */

/** @name SUP_IOCTL_LDR_OPEN
 * Open an image.
 * @{
 */
#define SUP_IOCTL_LDR_OPEN                              SUP_CTL_CODE_SIZE(3, SUP_IOCTL_LDR_OPEN_SIZE)
#define SUP_IOCTL_LDR_OPEN_SIZE                         sizeof(SUPLDROPEN)
#define SUP_IOCTL_LDR_OPEN_SIZE_IN                      sizeof(SUPLDROPEN)
#define SUP_IOCTL_LDR_OPEN_SIZE_OUT                     (sizeof(SUPREQHDR) + RT_SIZEOFMEMB(SUPLDROPEN, u.Out))
typedef struct SUPLDROPEN
{
    /** The header. */
    SUPREQHDR               Hdr;
    union
    {
        struct
        {
            /** Size of the image we'll be loading (including tables). */
            uint32_t        cbImageWithTabs;
            /** The size of the image bits. (Less or equal to cbImageWithTabs.) */
            uint32_t        cbImageBits;
            /** Image name.
             * This is the NAME of the image, not the file name. It is used
             * to share code with other processes. (Max len is 32 chars!)  */
            char            szName[32];
            /** Image file name.
             * This can be used to load the image using a native loader. */
            char            szFilename[260];
        } In;
        struct
        {
            /** The base address of the image. */
            uint64_t        pvImageBase;
            /** Indicate whether or not the image requires loading. */
            bool            fNeedsLoading;
            /** Indicates that we're using the native ring-0 loader. */
            bool            fNativeLoader;
        } Out;
    } u;
} SUPLDROPEN, *PSUPLDROPEN;
/** @} */

/**
 * SUPLDRLOAD::u::In::EP type.
 */
typedef enum SUPLDRLOADEP
{
    SUPLDRLOADEP_NOTHING = 0,
    SUPLDRLOADEP_VMMR0,
    SUPLDRLOADEP_SERVICE,
    SUPLDRLOADEP_32BIT_HACK = 0x7fffffff
} SUPLDRLOADEP;

typedef struct SUPLDRLOAD
{
    /** The header. */
    SUPREQHDR               Hdr;
    union
    {
        struct
        {
            /** The address of module initialization function. Similar to _DLL_InitTerm(hmod, 0). */
            uint64_t pfnModuleInit;
            /** The address of module termination function. Similar to _DLL_InitTerm(hmod, 1). */
            uint64_t pfnModuleTerm;
            /** Special entry points. */
            union
            {
                /** SUPLDRLOADEP_VMMR0. */
                struct
                {
                    /** The module handle (i.e. address). */
                    uint64_t                pvVMMR0;
                    /** Address of VMMR0EntryFast function. */
                    uint64_t                pvVMMR0EntryFast;
                    /** Address of VMMR0EntryEx function. */
                    uint64_t                pvVMMR0EntryEx;
                } VMMR0;
                /** SUPLDRLOADEP_SERVICE. */
                struct
                {
                    /** The service request handler.
                     * (PFNR0SERVICEREQHANDLER isn't defined yet.) */
                    uint64_t                pfnServiceReq;
                    /** Reserved, must be NIL. */
                    uint64_t                apvReserved[3];
                } Service;
            }               EP;
            /** Address. */
            uint64_t        pvImageBase;
            /** Entry point type. */
            SUPLDRLOADEP    eEPType;
            /** The size of the image bits (starting at offset 0 and
             * approaching offSymbols). */
            uint32_t        cbImageBits;
            /** The offset of the symbol table. */
            uint32_t        offSymbols;
            /** The number of entries in the symbol table. */
            uint32_t        cSymbols;
            /** The offset of the string table. */
            uint32_t        offStrTab;
            /** Size of the string table. */
            uint32_t        cbStrTab;
            /** Size of image data in achImage. */
            uint32_t        cbImageWithTabs;
            /** The image data. */
            uint8_t         abImage[1];
        } In;
        struct
        {
            /** Magic value indicating whether extended error information is
             * present or not (SUPLDRLOAD_ERROR_MAGIC). */
            uint64_t        uErrorMagic;
            /** Extended error information. */
            char            szError[2048];
        } Out;
    } u;
} SUPLDRLOAD, *PSUPLDRLOAD;
/** Magic value that indicates that there is a valid error information string
 * present on SUP_IOCTL_LDR_LOAD failure.
 * @remarks The value is choosen to be an unlikely init and term address. */
#define SUPLDRLOAD_ERROR_MAGIC      UINT64_C(0xabcdefef0feddcb9)
/** @} */