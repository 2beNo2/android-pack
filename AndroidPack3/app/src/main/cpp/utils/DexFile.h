//
// Created by huangchen on 2023/1/10.
//

#ifndef ANDROIDPACK_DEXFILE_H
#define ANDROIDPACK_DEXFILE_H

typedef uint8_t  u1;
typedef uint16_t  u2;
typedef uint32_t  u4;
typedef uint64_t  u8;

/*
 * gcc-style inline management -- ensures we have a copy of all functions
 * in the library, so code that links against us will work whether or not
 * it was built with optimizations enabled.
 */
#ifndef _DEX_GEN_INLINES             /* only defined by DexInlines.c */
# define DEX_INLINE extern __inline__
#else
# define DEX_INLINE
#endif

/* DEX file magic number */
#define DEX_MAGIC       "dex\n"

/* current version, encoded in 4 bytes of ASCII */
#define DEX_MAGIC_VERS  "036\0"

/*
 * older but still-recognized version (corresponding to Android API
 * levels 13 and earlier
 */
#define DEX_MAGIC_VERS_API_13  "035\0"

/* same, but for optimized DEX header */
#define DEX_OPT_MAGIC   "dey\n"
#define DEX_OPT_MAGIC_VERS  "036\0"

#define DEX_DEP_MAGIC   "deps"

/*
 * 160-bit SHA-1 digest.
 */
enum { kSHA1DigestLen = 20,
    kSHA1DigestOutputLen = kSHA1DigestLen*2 +1 };

/* general constants */
enum {
    kDexEndianConstant = 0x12345678,    /* the endianness indicator */
    kDexNoIndex = 0xffffffff,           /* not a valid index value */
};

/*
 * Enumeration of all the primitive types.
 */
enum PrimitiveType {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
};

/*
 * access flags and masks; the "standard" ones are all <= 0x4000
 *
 * Note: There are related declarations in vm/oo/Object.h in the ClassFlags
 * enum.
 */
enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
    0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
    (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
     | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
    (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
     | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
     | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
     | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
     | ACC_DECLARED_SYNCHRONIZED),
};

/* annotation constants */
enum {
    kDexVisibilityBuild         = 0x00,     /* annotation visibility */
    kDexVisibilityRuntime       = 0x01,
    kDexVisibilitySystem        = 0x02,

    kDexAnnotationByte          = 0x00,
    kDexAnnotationShort         = 0x02,
    kDexAnnotationChar          = 0x03,
    kDexAnnotationInt           = 0x04,
    kDexAnnotationLong          = 0x06,
    kDexAnnotationFloat         = 0x10,
    kDexAnnotationDouble        = 0x11,
    kDexAnnotationString        = 0x17,
    kDexAnnotationType          = 0x18,
    kDexAnnotationField         = 0x19,
    kDexAnnotationMethod        = 0x1a,
    kDexAnnotationEnum          = 0x1b,
    kDexAnnotationArray         = 0x1c,
    kDexAnnotationAnnotation    = 0x1d,
    kDexAnnotationNull          = 0x1e,
    kDexAnnotationBoolean       = 0x1f,

    kDexAnnotationValueTypeMask = 0x1f,     /* low 5 bits */
    kDexAnnotationValueArgShift = 5,
};

/* map item type codes */
enum {
    kDexTypeHeaderItem               = 0x0000,
    kDexTypeStringIdItem             = 0x0001,
    kDexTypeTypeIdItem               = 0x0002,
    kDexTypeProtoIdItem              = 0x0003,
    kDexTypeFieldIdItem              = 0x0004,
    kDexTypeMethodIdItem             = 0x0005,
    kDexTypeClassDefItem             = 0x0006,
    kDexTypeMapList                  = 0x1000,
    kDexTypeTypeList                 = 0x1001,
    kDexTypeAnnotationSetRefList     = 0x1002,
    kDexTypeAnnotationSetItem        = 0x1003,
    kDexTypeClassDataItem            = 0x2000,
    kDexTypeCodeItem                 = 0x2001,
    kDexTypeStringDataItem           = 0x2002,
    kDexTypeDebugInfoItem            = 0x2003,
    kDexTypeAnnotationItem           = 0x2004,
    kDexTypeEncodedArrayItem         = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};

/* auxillary data section chunk codes */
enum {
    kDexChunkClassLookup            = 0x434c4b50,   /* CLKP */
    kDexChunkRegisterMaps           = 0x524d4150,   /* RMAP */

    kDexChunkEnd                    = 0x41454e44,   /* AEND */
};

/* debug info opcodes and constants */
enum {
    DBG_END_SEQUENCE         = 0x00,
    DBG_ADVANCE_PC           = 0x01,
    DBG_ADVANCE_LINE         = 0x02,
    DBG_START_LOCAL          = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL            = 0x05,
    DBG_RESTART_LOCAL        = 0x06,
    DBG_SET_PROLOGUE_END     = 0x07,
    DBG_SET_EPILOGUE_BEGIN   = 0x08,
    DBG_SET_FILE             = 0x09,
    DBG_FIRST_SPECIAL        = 0x0a,
    DBG_LINE_BASE            = -4,
    DBG_LINE_RANGE           = 15,
};

/*
 * Direct-mapped "header_item" struct.
 */
struct DexHeader {
    u1  magic[8];           /* includes version number */
    u4  checksum;           /* adler32 checksum */
    u1  signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4  fileSize;           /* length of entire file */
    u4  headerSize;         /* offset to start of next section */
    u4  endianTag;
    u4  linkSize;
    u4  linkOff;
    u4  mapOff;
    u4  stringIdsSize;
    u4  stringIdsOff;
    u4  typeIdsSize;
    u4  typeIdsOff;
    u4  protoIdsSize;
    u4  protoIdsOff;
    u4  fieldIdsSize;
    u4  fieldIdsOff;
    u4  methodIdsSize;
    u4  methodIdsOff;
    u4  classDefsSize;
    u4  classDefsOff;
    u4  dataSize;
    u4  dataOff;
};

/*
 * Direct-mapped "map_item".
 */
struct DexMapItem {
    u2 type;              /* type code (see kDexType* above) */
    u2 unused;
    u4 size;              /* count of items of the indicated type */
    u4 offset;            /* file offset to the start of data */
};

/*
 * Direct-mapped "map_list".
 */
struct DexMapList {
    u4  size;               /* #of entries in list */
    DexMapItem list[1];     /* entries */
};

/*
 * Direct-mapped "string_id_item".
 */
struct DexStringId {
    u4 stringDataOff;      /* file offset to string_data_item */
};

/*
 * Direct-mapped "type_id_item".
 */
struct DexTypeId {
    u4  descriptorIdx;      /* index into stringIds list for type descriptor */
};

/*
 * Direct-mapped "field_id_item".
 */
struct DexFieldId {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  typeIdx;            /* index into typeIds for field type */
    u4  nameIdx;            /* index into stringIds for field name */
};

/*
 * Direct-mapped "method_id_item".
 */
struct DexMethodId {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  protoIdx;           /* index into protoIds for method prototype */
    u4  nameIdx;            /* index into stringIds for method name */
};

/*
 * Direct-mapped "proto_id_item".
 */
struct DexProtoId {
    u4  shortyIdx;          /* index into stringIds for shorty descriptor */
    u4  returnTypeIdx;      /* index into typeIds list for return type */
    u4  parametersOff;      /* file offset to type_list for parameter types */
};

/*
 * Direct-mapped "class_def_item".
 */
struct DexClassDef {
    u4  classIdx;           /* index into typeIds for this class */
    u4  accessFlags;
    u4  superclassIdx;      /* index into typeIds for superclass */
    u4  interfacesOff;      /* file offset to DexTypeList */
    u4  sourceFileIdx;      /* index into stringIds for source file name */
    u4  annotationsOff;     /* file offset to annotations_directory_item */
    u4  classDataOff;       /* file offset to class_data_item */
    u4  staticValuesOff;    /* file offset to DexEncodedArray */
};

/*
 * Direct-mapped "type_item".
 */
struct DexTypeItem {
    u2  typeIdx;            /* index into typeIds */
};

/*
 * Direct-mapped "type_list".
 */
struct DexTypeList {
    u4  size;               /* #of entries in list */
    DexTypeItem list[1];    /* entries */
};

/*
 * Direct-mapped "code_item".
 *
 * The "catches" table is used when throwing an exception,
 * "debugInfo" is used when displaying an exception stack trace or
 * debugging. An offset of zero indicates that there are no entries.
 */
struct DexCode {
    u2  registersSize;
    u2  insSize;
    u2  outsSize;
    u2  triesSize;
    u4  debugInfoOff;       /* file offset to debug info stream */
    u4  insnsSize;          /* size of the insns array, in u2 units */
    u2  insns[1];
    /* followed by optional u2 padding */
    /* followed by try_item[triesSize] */
    /* followed by uleb128 handlersSize */
    /* followed by catch_handler_item[handlersSize] */
};

/*
 * Direct-mapped "try_item".
 */
struct DexTry {
    u4  startAddr;          /* start address, in 16-bit code units */
    u2  insnCount;          /* instruction count, in 16-bit code units */
    u2  handlerOff;         /* offset in encoded handler data to handlers */
};

/*
 * Link table.  Currently undefined.
 */
struct DexLink {
    u1  bleargh;
};


/*
 * Direct-mapped "annotations_directory_item".
 */
struct DexAnnotationsDirectoryItem {
    u4  classAnnotationsOff;  /* offset to DexAnnotationSetItem */
    u4  fieldsSize;           /* count of DexFieldAnnotationsItem */
    u4  methodsSize;          /* count of DexMethodAnnotationsItem */
    u4  parametersSize;       /* count of DexParameterAnnotationsItem */
    /* followed by DexFieldAnnotationsItem[fieldsSize] */
    /* followed by DexMethodAnnotationsItem[methodsSize] */
    /* followed by DexParameterAnnotationsItem[parametersSize] */
};

/*
 * Direct-mapped "field_annotations_item".
 */
struct DexFieldAnnotationsItem {
    u4  fieldIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "method_annotations_item".
 */
struct DexMethodAnnotationsItem {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "parameter_annotations_item".
 */
struct DexParameterAnnotationsItem {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnotationSetRefList */
};

/*
 * Direct-mapped "annotation_set_ref_item".
 */
struct DexAnnotationSetRefItem {
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "annotation_set_ref_list".
 */
struct DexAnnotationSetRefList {
    u4  size;
    DexAnnotationSetRefItem list[1];
};

/*
 * Direct-mapped "annotation_set_item".
 */
struct DexAnnotationSetItem {
    u4  size;
    u4  entries[1];                 /* offset to DexAnnotationItem */
};

/*
 * Direct-mapped "annotation_item".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexAnnotationItem {
    u1  visibility;
    u1  annotation[1];              /* data in encoded_annotation format */
};

/*
 * Direct-mapped "encoded_array".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexEncodedArray {
    u1  array[1];                   /* data in encoded_array format */
};

/*
 * Lookup table for classes.  It provides a mapping from class name to
 * class definition.  Used by dexFindClass().
 *
 * We calculate this at DEX optimization time and embed it in the file so we
 * don't need the same hash table in every VM.  This is slightly slower than
 * a hash table with direct pointers to the items, but because it's shared
 * there's less of a penalty for using a fairly sparse table.
 */
struct DexClassLookup {
    int     size;                       // total size, including "size"
    int     numEntries;                 // size of table[]; always power of 2
    struct {
        u4      classDescriptorHash;    // class descriptor hash code
        int     classDescriptorOffset;  // in bytes, from start of DEX
        int     classDefOffset;         // in bytes, from start of DEX
    } table[1];
};

/*
 * Header added by DEX optimization pass.  Values are always written in
 * local byte and structure padding.  The first field (magic + version)
 * is guaranteed to be present and directly readable for all expected
 * compiler configurations; the rest is version-dependent.
 *
 * Try to keep this simple and fixed-size.
 */
struct DexOptHeader {
    u1  magic[8];           /* includes version number */

    u4  dexOffset;          /* file offset of DEX header */
    u4  dexLength;
    u4  depsOffset;         /* offset of optimized DEX dependency table */
    u4  depsLength;
    u4  optOffset;          /* file offset of optimized data tables */
    u4  optLength;

    u4  flags;              /* some info flags */
    u4  checksum;           /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};

#define DEX_OPT_FLAG_BIG            (1<<1)  /* swapped to big-endian */

#define DEX_INTERFACE_CACHE_SIZE    128     /* must be power of 2 */

/*
 * Structure representing a DEX file.
 *
 * Code should regard DexFile as opaque, using the API calls provided here
 * to access specific structures.
 */
struct DexFile {
    /* directly-mapped "opt" header */
    const DexOptHeader* pOptHeader;

    /* pointers to directly-mapped structs and arrays in base DEX */
    const DexHeader*    pHeader;
    const DexStringId*  pStringIds;
    const DexTypeId*    pTypeIds;
    const DexFieldId*   pFieldIds;
    const DexMethodId*  pMethodIds;
    const DexProtoId*   pProtoIds;
    const DexClassDef*  pClassDefs;
    const DexLink*      pLinkData;

    /*
     * These are mapped out of the "auxillary" section, and may not be
     * included in the file.
     */
    const DexClassLookup* pClassLookup;
    const void*         pRegisterMapPool;       // RegisterMapClassPool

    /* points to start of DEX file data */
    const u1*           baseAddr;

    /* track memory overhead for auxillary structures */
    int                 overhead;

    /* additional app-specific data structures associated with the DEX */
    //void*               auxData;
};


enum Opcode {
    // BEGIN(libdex-opcode-enum); GENERATED AUTOMATICALLY BY opcode-gen
    OP_NOP                          = 0x00,
    OP_MOVE                         = 0x01,
    OP_MOVE_FROM16                  = 0x02,
    OP_MOVE_16                      = 0x03,
    OP_MOVE_WIDE                    = 0x04,
    OP_MOVE_WIDE_FROM16             = 0x05,
    OP_MOVE_WIDE_16                 = 0x06,
    OP_MOVE_OBJECT                  = 0x07,
    OP_MOVE_OBJECT_FROM16           = 0x08,
    OP_MOVE_OBJECT_16               = 0x09,
    OP_MOVE_RESULT                  = 0x0a,
    OP_MOVE_RESULT_WIDE             = 0x0b,
    OP_MOVE_RESULT_OBJECT           = 0x0c,
    OP_MOVE_EXCEPTION               = 0x0d,
    OP_RETURN_VOID                  = 0x0e,
    OP_RETURN                       = 0x0f,
    OP_RETURN_WIDE                  = 0x10,
    OP_RETURN_OBJECT                = 0x11,
    OP_CONST_4                      = 0x12,
    OP_CONST_16                     = 0x13,
    OP_CONST                        = 0x14,
    OP_CONST_HIGH16                 = 0x15,
    OP_CONST_WIDE_16                = 0x16,
    OP_CONST_WIDE_32                = 0x17,
    OP_CONST_WIDE                   = 0x18,
    OP_CONST_WIDE_HIGH16            = 0x19,
    OP_CONST_STRING                 = 0x1a,
    OP_CONST_STRING_JUMBO           = 0x1b,
    OP_CONST_CLASS                  = 0x1c,
    OP_MONITOR_ENTER                = 0x1d,
    OP_MONITOR_EXIT                 = 0x1e,
    OP_CHECK_CAST                   = 0x1f,
    OP_INSTANCE_OF                  = 0x20,
    OP_ARRAY_LENGTH                 = 0x21,
    OP_NEW_INSTANCE                 = 0x22,
    OP_NEW_ARRAY                    = 0x23,
    OP_FILLED_NEW_ARRAY             = 0x24,
    OP_FILLED_NEW_ARRAY_RANGE       = 0x25,
    OP_FILL_ARRAY_DATA              = 0x26,
    OP_THROW                        = 0x27,
    OP_GOTO                         = 0x28,
    OP_GOTO_16                      = 0x29,
    OP_GOTO_32                      = 0x2a,
    OP_PACKED_SWITCH                = 0x2b,
    OP_SPARSE_SWITCH                = 0x2c,
    OP_CMPL_FLOAT                   = 0x2d,
    OP_CMPG_FLOAT                   = 0x2e,
    OP_CMPL_DOUBLE                  = 0x2f,
    OP_CMPG_DOUBLE                  = 0x30,
    OP_CMP_LONG                     = 0x31,
    OP_IF_EQ                        = 0x32,
    OP_IF_NE                        = 0x33,
    OP_IF_LT                        = 0x34,
    OP_IF_GE                        = 0x35,
    OP_IF_GT                        = 0x36,
    OP_IF_LE                        = 0x37,
    OP_IF_EQZ                       = 0x38,
    OP_IF_NEZ                       = 0x39,
    OP_IF_LTZ                       = 0x3a,
    OP_IF_GEZ                       = 0x3b,
    OP_IF_GTZ                       = 0x3c,
    OP_IF_LEZ                       = 0x3d,
    OP_UNUSED_3E                    = 0x3e,
    OP_UNUSED_3F                    = 0x3f,
    OP_UNUSED_40                    = 0x40,
    OP_UNUSED_41                    = 0x41,
    OP_UNUSED_42                    = 0x42,
    OP_UNUSED_43                    = 0x43,
    OP_AGET                         = 0x44,
    OP_AGET_WIDE                    = 0x45,
    OP_AGET_OBJECT                  = 0x46,
    OP_AGET_BOOLEAN                 = 0x47,
    OP_AGET_BYTE                    = 0x48,
    OP_AGET_CHAR                    = 0x49,
    OP_AGET_SHORT                   = 0x4a,
    OP_APUT                         = 0x4b,
    OP_APUT_WIDE                    = 0x4c,
    OP_APUT_OBJECT                  = 0x4d,
    OP_APUT_BOOLEAN                 = 0x4e,
    OP_APUT_BYTE                    = 0x4f,
    OP_APUT_CHAR                    = 0x50,
    OP_APUT_SHORT                   = 0x51,
    OP_IGET                         = 0x52,
    OP_IGET_WIDE                    = 0x53,
    OP_IGET_OBJECT                  = 0x54,
    OP_IGET_BOOLEAN                 = 0x55,
    OP_IGET_BYTE                    = 0x56,
    OP_IGET_CHAR                    = 0x57,
    OP_IGET_SHORT                   = 0x58,
    OP_IPUT                         = 0x59,
    OP_IPUT_WIDE                    = 0x5a,
    OP_IPUT_OBJECT                  = 0x5b,
    OP_IPUT_BOOLEAN                 = 0x5c,
    OP_IPUT_BYTE                    = 0x5d,
    OP_IPUT_CHAR                    = 0x5e,
    OP_IPUT_SHORT                   = 0x5f,
    OP_SGET                         = 0x60,
    OP_SGET_WIDE                    = 0x61,
    OP_SGET_OBJECT                  = 0x62,
    OP_SGET_BOOLEAN                 = 0x63,
    OP_SGET_BYTE                    = 0x64,
    OP_SGET_CHAR                    = 0x65,
    OP_SGET_SHORT                   = 0x66,
    OP_SPUT                         = 0x67,
    OP_SPUT_WIDE                    = 0x68,
    OP_SPUT_OBJECT                  = 0x69,
    OP_SPUT_BOOLEAN                 = 0x6a,
    OP_SPUT_BYTE                    = 0x6b,
    OP_SPUT_CHAR                    = 0x6c,
    OP_SPUT_SHORT                   = 0x6d,
    OP_INVOKE_VIRTUAL               = 0x6e,
    OP_INVOKE_SUPER                 = 0x6f,
    OP_INVOKE_DIRECT                = 0x70,
    OP_INVOKE_STATIC                = 0x71,
    OP_INVOKE_INTERFACE             = 0x72,
    OP_UNUSED_73                    = 0x73,
    OP_INVOKE_VIRTUAL_RANGE         = 0x74,
    OP_INVOKE_SUPER_RANGE           = 0x75,
    OP_INVOKE_DIRECT_RANGE          = 0x76,
    OP_INVOKE_STATIC_RANGE          = 0x77,
    OP_INVOKE_INTERFACE_RANGE       = 0x78,
    OP_UNUSED_79                    = 0x79,
    OP_UNUSED_7A                    = 0x7a,
    OP_NEG_INT                      = 0x7b,
    OP_NOT_INT                      = 0x7c,
    OP_NEG_LONG                     = 0x7d,
    OP_NOT_LONG                     = 0x7e,
    OP_NEG_FLOAT                    = 0x7f,
    OP_NEG_DOUBLE                   = 0x80,
    OP_INT_TO_LONG                  = 0x81,
    OP_INT_TO_FLOAT                 = 0x82,
    OP_INT_TO_DOUBLE                = 0x83,
    OP_LONG_TO_INT                  = 0x84,
    OP_LONG_TO_FLOAT                = 0x85,
    OP_LONG_TO_DOUBLE               = 0x86,
    OP_FLOAT_TO_INT                 = 0x87,
    OP_FLOAT_TO_LONG                = 0x88,
    OP_FLOAT_TO_DOUBLE              = 0x89,
    OP_DOUBLE_TO_INT                = 0x8a,
    OP_DOUBLE_TO_LONG               = 0x8b,
    OP_DOUBLE_TO_FLOAT              = 0x8c,
    OP_INT_TO_BYTE                  = 0x8d,
    OP_INT_TO_CHAR                  = 0x8e,
    OP_INT_TO_SHORT                 = 0x8f,
    OP_ADD_INT                      = 0x90,
    OP_SUB_INT                      = 0x91,
    OP_MUL_INT                      = 0x92,
    OP_DIV_INT                      = 0x93,
    OP_REM_INT                      = 0x94,
    OP_AND_INT                      = 0x95,
    OP_OR_INT                       = 0x96,
    OP_XOR_INT                      = 0x97,
    OP_SHL_INT                      = 0x98,
    OP_SHR_INT                      = 0x99,
    OP_USHR_INT                     = 0x9a,
    OP_ADD_LONG                     = 0x9b,
    OP_SUB_LONG                     = 0x9c,
    OP_MUL_LONG                     = 0x9d,
    OP_DIV_LONG                     = 0x9e,
    OP_REM_LONG                     = 0x9f,
    OP_AND_LONG                     = 0xa0,
    OP_OR_LONG                      = 0xa1,
    OP_XOR_LONG                     = 0xa2,
    OP_SHL_LONG                     = 0xa3,
    OP_SHR_LONG                     = 0xa4,
    OP_USHR_LONG                    = 0xa5,
    OP_ADD_FLOAT                    = 0xa6,
    OP_SUB_FLOAT                    = 0xa7,
    OP_MUL_FLOAT                    = 0xa8,
    OP_DIV_FLOAT                    = 0xa9,
    OP_REM_FLOAT                    = 0xaa,
    OP_ADD_DOUBLE                   = 0xab,
    OP_SUB_DOUBLE                   = 0xac,
    OP_MUL_DOUBLE                   = 0xad,
    OP_DIV_DOUBLE                   = 0xae,
    OP_REM_DOUBLE                   = 0xaf,
    OP_ADD_INT_2ADDR                = 0xb0,
    OP_SUB_INT_2ADDR                = 0xb1,
    OP_MUL_INT_2ADDR                = 0xb2,
    OP_DIV_INT_2ADDR                = 0xb3,
    OP_REM_INT_2ADDR                = 0xb4,
    OP_AND_INT_2ADDR                = 0xb5,
    OP_OR_INT_2ADDR                 = 0xb6,
    OP_XOR_INT_2ADDR                = 0xb7,
    OP_SHL_INT_2ADDR                = 0xb8,
    OP_SHR_INT_2ADDR                = 0xb9,
    OP_USHR_INT_2ADDR               = 0xba,
    OP_ADD_LONG_2ADDR               = 0xbb,
    OP_SUB_LONG_2ADDR               = 0xbc,
    OP_MUL_LONG_2ADDR               = 0xbd,
    OP_DIV_LONG_2ADDR               = 0xbe,
    OP_REM_LONG_2ADDR               = 0xbf,
    OP_AND_LONG_2ADDR               = 0xc0,
    OP_OR_LONG_2ADDR                = 0xc1,
    OP_XOR_LONG_2ADDR               = 0xc2,
    OP_SHL_LONG_2ADDR               = 0xc3,
    OP_SHR_LONG_2ADDR               = 0xc4,
    OP_USHR_LONG_2ADDR              = 0xc5,
    OP_ADD_FLOAT_2ADDR              = 0xc6,
    OP_SUB_FLOAT_2ADDR              = 0xc7,
    OP_MUL_FLOAT_2ADDR              = 0xc8,
    OP_DIV_FLOAT_2ADDR              = 0xc9,
    OP_REM_FLOAT_2ADDR              = 0xca,
    OP_ADD_DOUBLE_2ADDR             = 0xcb,
    OP_SUB_DOUBLE_2ADDR             = 0xcc,
    OP_MUL_DOUBLE_2ADDR             = 0xcd,
    OP_DIV_DOUBLE_2ADDR             = 0xce,
    OP_REM_DOUBLE_2ADDR             = 0xcf,
    OP_ADD_INT_LIT16                = 0xd0,
    OP_RSUB_INT                     = 0xd1,
    OP_MUL_INT_LIT16                = 0xd2,
    OP_DIV_INT_LIT16                = 0xd3,
    OP_REM_INT_LIT16                = 0xd4,
    OP_AND_INT_LIT16                = 0xd5,
    OP_OR_INT_LIT16                 = 0xd6,
    OP_XOR_INT_LIT16                = 0xd7,
    OP_ADD_INT_LIT8                 = 0xd8,
    OP_RSUB_INT_LIT8                = 0xd9,
    OP_MUL_INT_LIT8                 = 0xda,
    OP_DIV_INT_LIT8                 = 0xdb,
    OP_REM_INT_LIT8                 = 0xdc,
    OP_AND_INT_LIT8                 = 0xdd,
    OP_OR_INT_LIT8                  = 0xde,
    OP_XOR_INT_LIT8                 = 0xdf,
    OP_SHL_INT_LIT8                 = 0xe0,
    OP_SHR_INT_LIT8                 = 0xe1,
    OP_USHR_INT_LIT8                = 0xe2,
    OP_IGET_VOLATILE                = 0xe3,
    OP_IPUT_VOLATILE                = 0xe4,
    OP_SGET_VOLATILE                = 0xe5,
    OP_SPUT_VOLATILE                = 0xe6,
    OP_IGET_OBJECT_VOLATILE         = 0xe7,
    OP_IGET_WIDE_VOLATILE           = 0xe8,
    OP_IPUT_WIDE_VOLATILE           = 0xe9,
    OP_SGET_WIDE_VOLATILE           = 0xea,
    OP_SPUT_WIDE_VOLATILE           = 0xeb,
    OP_BREAKPOINT                   = 0xec,
    OP_THROW_VERIFICATION_ERROR     = 0xed,
    OP_EXECUTE_INLINE               = 0xee,
    OP_EXECUTE_INLINE_RANGE         = 0xef,
    OP_INVOKE_OBJECT_INIT_RANGE     = 0xf0,
    OP_RETURN_VOID_BARRIER          = 0xf1,
    OP_IGET_QUICK                   = 0xf2,
    OP_IGET_WIDE_QUICK              = 0xf3,
    OP_IGET_OBJECT_QUICK            = 0xf4,
    OP_IPUT_QUICK                   = 0xf5,
    OP_IPUT_WIDE_QUICK              = 0xf6,
    OP_IPUT_OBJECT_QUICK            = 0xf7,
    OP_INVOKE_VIRTUAL_QUICK         = 0xf8,
    OP_INVOKE_VIRTUAL_QUICK_RANGE   = 0xf9,
    OP_INVOKE_SUPER_QUICK           = 0xfa,
    OP_INVOKE_SUPER_QUICK_RANGE     = 0xfb,
    OP_IPUT_OBJECT_VOLATILE         = 0xfc,
    OP_SGET_OBJECT_VOLATILE         = 0xfd,
    OP_SPUT_OBJECT_VOLATILE         = 0xfe,
    OP_DISPATCH_FF                  = 0xff,
    OP_CONST_CLASS_JUMBO            = 0x100,
    OP_CHECK_CAST_JUMBO             = 0x101,
    OP_INSTANCE_OF_JUMBO            = 0x102,
    OP_NEW_INSTANCE_JUMBO           = 0x103,
    OP_NEW_ARRAY_JUMBO              = 0x104,
    OP_FILLED_NEW_ARRAY_JUMBO       = 0x105,
    OP_IGET_JUMBO                   = 0x106,
    OP_IGET_WIDE_JUMBO              = 0x107,
    OP_IGET_OBJECT_JUMBO            = 0x108,
    OP_IGET_BOOLEAN_JUMBO           = 0x109,
    OP_IGET_BYTE_JUMBO              = 0x10a,
    OP_IGET_CHAR_JUMBO              = 0x10b,
    OP_IGET_SHORT_JUMBO             = 0x10c,
    OP_IPUT_JUMBO                   = 0x10d,
    OP_IPUT_WIDE_JUMBO              = 0x10e,
    OP_IPUT_OBJECT_JUMBO            = 0x10f,
    OP_IPUT_BOOLEAN_JUMBO           = 0x110,
    OP_IPUT_BYTE_JUMBO              = 0x111,
    OP_IPUT_CHAR_JUMBO              = 0x112,
    OP_IPUT_SHORT_JUMBO             = 0x113,
    OP_SGET_JUMBO                   = 0x114,
    OP_SGET_WIDE_JUMBO              = 0x115,
    OP_SGET_OBJECT_JUMBO            = 0x116,
    OP_SGET_BOOLEAN_JUMBO           = 0x117,
    OP_SGET_BYTE_JUMBO              = 0x118,
    OP_SGET_CHAR_JUMBO              = 0x119,
    OP_SGET_SHORT_JUMBO             = 0x11a,
    OP_SPUT_JUMBO                   = 0x11b,
    OP_SPUT_WIDE_JUMBO              = 0x11c,
    OP_SPUT_OBJECT_JUMBO            = 0x11d,
    OP_SPUT_BOOLEAN_JUMBO           = 0x11e,
    OP_SPUT_BYTE_JUMBO              = 0x11f,
    OP_SPUT_CHAR_JUMBO              = 0x120,
    OP_SPUT_SHORT_JUMBO             = 0x121,
    OP_INVOKE_VIRTUAL_JUMBO         = 0x122,
    OP_INVOKE_SUPER_JUMBO           = 0x123,
    OP_INVOKE_DIRECT_JUMBO          = 0x124,
    OP_INVOKE_STATIC_JUMBO          = 0x125,
    OP_INVOKE_INTERFACE_JUMBO       = 0x126,
    OP_UNUSED_27FF                  = 0x127,
    OP_UNUSED_28FF                  = 0x128,
    OP_UNUSED_29FF                  = 0x129,
    OP_UNUSED_2AFF                  = 0x12a,
    OP_UNUSED_2BFF                  = 0x12b,
    OP_UNUSED_2CFF                  = 0x12c,
    OP_UNUSED_2DFF                  = 0x12d,
    OP_UNUSED_2EFF                  = 0x12e,
    OP_UNUSED_2FFF                  = 0x12f,
    OP_UNUSED_30FF                  = 0x130,
    OP_UNUSED_31FF                  = 0x131,
    OP_UNUSED_32FF                  = 0x132,
    OP_UNUSED_33FF                  = 0x133,
    OP_UNUSED_34FF                  = 0x134,
    OP_UNUSED_35FF                  = 0x135,
    OP_UNUSED_36FF                  = 0x136,
    OP_UNUSED_37FF                  = 0x137,
    OP_UNUSED_38FF                  = 0x138,
    OP_UNUSED_39FF                  = 0x139,
    OP_UNUSED_3AFF                  = 0x13a,
    OP_UNUSED_3BFF                  = 0x13b,
    OP_UNUSED_3CFF                  = 0x13c,
    OP_UNUSED_3DFF                  = 0x13d,
    OP_UNUSED_3EFF                  = 0x13e,
    OP_UNUSED_3FFF                  = 0x13f,
    OP_UNUSED_40FF                  = 0x140,
    OP_UNUSED_41FF                  = 0x141,
    OP_UNUSED_42FF                  = 0x142,
    OP_UNUSED_43FF                  = 0x143,
    OP_UNUSED_44FF                  = 0x144,
    OP_UNUSED_45FF                  = 0x145,
    OP_UNUSED_46FF                  = 0x146,
    OP_UNUSED_47FF                  = 0x147,
    OP_UNUSED_48FF                  = 0x148,
    OP_UNUSED_49FF                  = 0x149,
    OP_UNUSED_4AFF                  = 0x14a,
    OP_UNUSED_4BFF                  = 0x14b,
    OP_UNUSED_4CFF                  = 0x14c,
    OP_UNUSED_4DFF                  = 0x14d,
    OP_UNUSED_4EFF                  = 0x14e,
    OP_UNUSED_4FFF                  = 0x14f,
    OP_UNUSED_50FF                  = 0x150,
    OP_UNUSED_51FF                  = 0x151,
    OP_UNUSED_52FF                  = 0x152,
    OP_UNUSED_53FF                  = 0x153,
    OP_UNUSED_54FF                  = 0x154,
    OP_UNUSED_55FF                  = 0x155,
    OP_UNUSED_56FF                  = 0x156,
    OP_UNUSED_57FF                  = 0x157,
    OP_UNUSED_58FF                  = 0x158,
    OP_UNUSED_59FF                  = 0x159,
    OP_UNUSED_5AFF                  = 0x15a,
    OP_UNUSED_5BFF                  = 0x15b,
    OP_UNUSED_5CFF                  = 0x15c,
    OP_UNUSED_5DFF                  = 0x15d,
    OP_UNUSED_5EFF                  = 0x15e,
    OP_UNUSED_5FFF                  = 0x15f,
    OP_UNUSED_60FF                  = 0x160,
    OP_UNUSED_61FF                  = 0x161,
    OP_UNUSED_62FF                  = 0x162,
    OP_UNUSED_63FF                  = 0x163,
    OP_UNUSED_64FF                  = 0x164,
    OP_UNUSED_65FF                  = 0x165,
    OP_UNUSED_66FF                  = 0x166,
    OP_UNUSED_67FF                  = 0x167,
    OP_UNUSED_68FF                  = 0x168,
    OP_UNUSED_69FF                  = 0x169,
    OP_UNUSED_6AFF                  = 0x16a,
    OP_UNUSED_6BFF                  = 0x16b,
    OP_UNUSED_6CFF                  = 0x16c,
    OP_UNUSED_6DFF                  = 0x16d,
    OP_UNUSED_6EFF                  = 0x16e,
    OP_UNUSED_6FFF                  = 0x16f,
    OP_UNUSED_70FF                  = 0x170,
    OP_UNUSED_71FF                  = 0x171,
    OP_UNUSED_72FF                  = 0x172,
    OP_UNUSED_73FF                  = 0x173,
    OP_UNUSED_74FF                  = 0x174,
    OP_UNUSED_75FF                  = 0x175,
    OP_UNUSED_76FF                  = 0x176,
    OP_UNUSED_77FF                  = 0x177,
    OP_UNUSED_78FF                  = 0x178,
    OP_UNUSED_79FF                  = 0x179,
    OP_UNUSED_7AFF                  = 0x17a,
    OP_UNUSED_7BFF                  = 0x17b,
    OP_UNUSED_7CFF                  = 0x17c,
    OP_UNUSED_7DFF                  = 0x17d,
    OP_UNUSED_7EFF                  = 0x17e,
    OP_UNUSED_7FFF                  = 0x17f,
    OP_UNUSED_80FF                  = 0x180,
    OP_UNUSED_81FF                  = 0x181,
    OP_UNUSED_82FF                  = 0x182,
    OP_UNUSED_83FF                  = 0x183,
    OP_UNUSED_84FF                  = 0x184,
    OP_UNUSED_85FF                  = 0x185,
    OP_UNUSED_86FF                  = 0x186,
    OP_UNUSED_87FF                  = 0x187,
    OP_UNUSED_88FF                  = 0x188,
    OP_UNUSED_89FF                  = 0x189,
    OP_UNUSED_8AFF                  = 0x18a,
    OP_UNUSED_8BFF                  = 0x18b,
    OP_UNUSED_8CFF                  = 0x18c,
    OP_UNUSED_8DFF                  = 0x18d,
    OP_UNUSED_8EFF                  = 0x18e,
    OP_UNUSED_8FFF                  = 0x18f,
    OP_UNUSED_90FF                  = 0x190,
    OP_UNUSED_91FF                  = 0x191,
    OP_UNUSED_92FF                  = 0x192,
    OP_UNUSED_93FF                  = 0x193,
    OP_UNUSED_94FF                  = 0x194,
    OP_UNUSED_95FF                  = 0x195,
    OP_UNUSED_96FF                  = 0x196,
    OP_UNUSED_97FF                  = 0x197,
    OP_UNUSED_98FF                  = 0x198,
    OP_UNUSED_99FF                  = 0x199,
    OP_UNUSED_9AFF                  = 0x19a,
    OP_UNUSED_9BFF                  = 0x19b,
    OP_UNUSED_9CFF                  = 0x19c,
    OP_UNUSED_9DFF                  = 0x19d,
    OP_UNUSED_9EFF                  = 0x19e,
    OP_UNUSED_9FFF                  = 0x19f,
    OP_UNUSED_A0FF                  = 0x1a0,
    OP_UNUSED_A1FF                  = 0x1a1,
    OP_UNUSED_A2FF                  = 0x1a2,
    OP_UNUSED_A3FF                  = 0x1a3,
    OP_UNUSED_A4FF                  = 0x1a4,
    OP_UNUSED_A5FF                  = 0x1a5,
    OP_UNUSED_A6FF                  = 0x1a6,
    OP_UNUSED_A7FF                  = 0x1a7,
    OP_UNUSED_A8FF                  = 0x1a8,
    OP_UNUSED_A9FF                  = 0x1a9,
    OP_UNUSED_AAFF                  = 0x1aa,
    OP_UNUSED_ABFF                  = 0x1ab,
    OP_UNUSED_ACFF                  = 0x1ac,
    OP_UNUSED_ADFF                  = 0x1ad,
    OP_UNUSED_AEFF                  = 0x1ae,
    OP_UNUSED_AFFF                  = 0x1af,
    OP_UNUSED_B0FF                  = 0x1b0,
    OP_UNUSED_B1FF                  = 0x1b1,
    OP_UNUSED_B2FF                  = 0x1b2,
    OP_UNUSED_B3FF                  = 0x1b3,
    OP_UNUSED_B4FF                  = 0x1b4,
    OP_UNUSED_B5FF                  = 0x1b5,
    OP_UNUSED_B6FF                  = 0x1b6,
    OP_UNUSED_B7FF                  = 0x1b7,
    OP_UNUSED_B8FF                  = 0x1b8,
    OP_UNUSED_B9FF                  = 0x1b9,
    OP_UNUSED_BAFF                  = 0x1ba,
    OP_UNUSED_BBFF                  = 0x1bb,
    OP_UNUSED_BCFF                  = 0x1bc,
    OP_UNUSED_BDFF                  = 0x1bd,
    OP_UNUSED_BEFF                  = 0x1be,
    OP_UNUSED_BFFF                  = 0x1bf,
    OP_UNUSED_C0FF                  = 0x1c0,
    OP_UNUSED_C1FF                  = 0x1c1,
    OP_UNUSED_C2FF                  = 0x1c2,
    OP_UNUSED_C3FF                  = 0x1c3,
    OP_UNUSED_C4FF                  = 0x1c4,
    OP_UNUSED_C5FF                  = 0x1c5,
    OP_UNUSED_C6FF                  = 0x1c6,
    OP_UNUSED_C7FF                  = 0x1c7,
    OP_UNUSED_C8FF                  = 0x1c8,
    OP_UNUSED_C9FF                  = 0x1c9,
    OP_UNUSED_CAFF                  = 0x1ca,
    OP_UNUSED_CBFF                  = 0x1cb,
    OP_UNUSED_CCFF                  = 0x1cc,
    OP_UNUSED_CDFF                  = 0x1cd,
    OP_UNUSED_CEFF                  = 0x1ce,
    OP_UNUSED_CFFF                  = 0x1cf,
    OP_UNUSED_D0FF                  = 0x1d0,
    OP_UNUSED_D1FF                  = 0x1d1,
    OP_UNUSED_D2FF                  = 0x1d2,
    OP_UNUSED_D3FF                  = 0x1d3,
    OP_UNUSED_D4FF                  = 0x1d4,
    OP_UNUSED_D5FF                  = 0x1d5,
    OP_UNUSED_D6FF                  = 0x1d6,
    OP_UNUSED_D7FF                  = 0x1d7,
    OP_UNUSED_D8FF                  = 0x1d8,
    OP_UNUSED_D9FF                  = 0x1d9,
    OP_UNUSED_DAFF                  = 0x1da,
    OP_UNUSED_DBFF                  = 0x1db,
    OP_UNUSED_DCFF                  = 0x1dc,
    OP_UNUSED_DDFF                  = 0x1dd,
    OP_UNUSED_DEFF                  = 0x1de,
    OP_UNUSED_DFFF                  = 0x1df,
    OP_UNUSED_E0FF                  = 0x1e0,
    OP_UNUSED_E1FF                  = 0x1e1,
    OP_UNUSED_E2FF                  = 0x1e2,
    OP_UNUSED_E3FF                  = 0x1e3,
    OP_UNUSED_E4FF                  = 0x1e4,
    OP_UNUSED_E5FF                  = 0x1e5,
    OP_UNUSED_E6FF                  = 0x1e6,
    OP_UNUSED_E7FF                  = 0x1e7,
    OP_UNUSED_E8FF                  = 0x1e8,
    OP_UNUSED_E9FF                  = 0x1e9,
    OP_UNUSED_EAFF                  = 0x1ea,
    OP_UNUSED_EBFF                  = 0x1eb,
    OP_UNUSED_ECFF                  = 0x1ec,
    OP_UNUSED_EDFF                  = 0x1ed,
    OP_UNUSED_EEFF                  = 0x1ee,
    OP_UNUSED_EFFF                  = 0x1ef,
    OP_UNUSED_F0FF                  = 0x1f0,
    OP_UNUSED_F1FF                  = 0x1f1,
    OP_INVOKE_OBJECT_INIT_JUMBO     = 0x1f2,
    OP_IGET_VOLATILE_JUMBO          = 0x1f3,
    OP_IGET_WIDE_VOLATILE_JUMBO     = 0x1f4,
    OP_IGET_OBJECT_VOLATILE_JUMBO   = 0x1f5,
    OP_IPUT_VOLATILE_JUMBO          = 0x1f6,
    OP_IPUT_WIDE_VOLATILE_JUMBO     = 0x1f7,
    OP_IPUT_OBJECT_VOLATILE_JUMBO   = 0x1f8,
    OP_SGET_VOLATILE_JUMBO          = 0x1f9,
    OP_SGET_WIDE_VOLATILE_JUMBO     = 0x1fa,
    OP_SGET_OBJECT_VOLATILE_JUMBO   = 0x1fb,
    OP_SPUT_VOLATILE_JUMBO          = 0x1fc,
    OP_SPUT_WIDE_VOLATILE_JUMBO     = 0x1fd,
    OP_SPUT_OBJECT_VOLATILE_JUMBO   = 0x1fe,
    OP_THROW_VERIFICATION_ERROR_JUMBO = 0x1ff,
    // END(libdex-opcode-enum)
};


#endif //ANDROIDPACK_DEXFILE_H
