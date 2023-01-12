//
// Created by huangchen on 2023/1/12.
//

#ifndef AOSP_DEXFILE_H
#define AOSP_DEXFILE_H


#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;  \
  void operator=(const TypeName&) = delete


namespace art{

class DexFile {
public:
    static const size_t kSha1DigestSize = 20;
    static const uint32_t kDexEndianConstant = 0x12345678;

    // name of the DexFile entry within a zip archive
    static const char* kClassesDex;

    // The value of an invalid index.
    static const uint32_t kDexNoIndex = 0xFFFFFFFF;

    // The value of an invalid index.
    static const uint16_t kDexNoIndex16 = 0xFFFF;

    // The separator charactor in MultiDex locations.
    static constexpr char kMultiDexSeparator = ':';

    // Raw header_item.
    struct Header {
        uint8_t magic_[8];
        uint32_t checksum_;  // See also location_checksum_
        uint8_t signature_[kSha1DigestSize];
        uint32_t file_size_;  // size of entire file
        uint32_t header_size_;  // offset to start of next section
        uint32_t endian_tag_;
        uint32_t link_size_;  // unused
        uint32_t link_off_;  // unused
        uint32_t map_off_;  // unused
        uint32_t string_ids_size_;  // number of StringIds
        uint32_t string_ids_off_;  // file offset of StringIds array
        uint32_t type_ids_size_;  // number of TypeIds, we don't support more than 65535
        uint32_t type_ids_off_;  // file offset of TypeIds array
        uint32_t proto_ids_size_;  // number of ProtoIds, we don't support more than 65535
        uint32_t proto_ids_off_;  // file offset of ProtoIds array
        uint32_t field_ids_size_;  // number of FieldIds
        uint32_t field_ids_off_;  // file offset of FieldIds array
        uint32_t method_ids_size_;  // number of MethodIds
        uint32_t method_ids_off_;  // file offset of MethodIds array
        uint32_t class_defs_size_;  // number of ClassDefs
        uint32_t class_defs_off_;  // file offset of ClassDef array
        uint32_t data_size_;  // unused
        uint32_t data_off_;  // unused

    private:
        DISALLOW_COPY_AND_ASSIGN(Header);
    };

    // Raw code_item.
    struct CodeItem {
        uint16_t registers_size_;
        uint16_t ins_size_;
        uint16_t outs_size_;
        uint16_t tries_size_;
        uint32_t debug_info_off_;  // file offset to debug info stream
        uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
        uint16_t insns_[1];

    private:
        DISALLOW_COPY_AND_ASSIGN(CodeItem);
    };
    virtual ~DexFile(){

    }
    const uint8_t* const begin_;
    const size_t size_;
};


class ArtMethod{
public:
    uint32_t declaring_class_[5];
    uint64_t entry_point_from_interpreter_;
    uint64_t entry_point_from_jni_;

    uint64_t entry_point_from_quick_compiled_code_;
    uint64_t gc_map_;
    uint32_t access_flags_;
    uint32_t dex_code_item_offset_;
    uint32_t dex_method_index_;
    uint32_t method_index_;
};

} //namespace art
#endif //AOSP_DEXFILE_H
