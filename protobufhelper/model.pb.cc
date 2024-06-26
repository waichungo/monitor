// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: model.proto
// Protobuf C++ Version: 5.28.0-dev

#include "model.pb.h"

#include <algorithm>
#include <type_traits>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/generated_message_tctable_impl.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace binmessage {

inline constexpr BinaryMessage::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : recepient_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        sender_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        meta_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        data_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        compressed_{false},
        code_{0},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR BinaryMessage::BinaryMessage(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct BinaryMessageDefaultTypeInternal {
  PROTOBUF_CONSTEXPR BinaryMessageDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~BinaryMessageDefaultTypeInternal() {}
  union {
    BinaryMessage _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 BinaryMessageDefaultTypeInternal _BinaryMessage_default_instance_;
}  // namespace binmessage
static constexpr const ::_pb::EnumDescriptor**
    file_level_enum_descriptors_model_2eproto = nullptr;
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_model_2eproto = nullptr;
const ::uint32_t
    TableStruct_model_2eproto::offsets[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
        protodesc_cold) = {
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.compressed_),
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.code_),
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.recepient_),
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.sender_),
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.meta_),
        PROTOBUF_FIELD_OFFSET(::binmessage::BinaryMessage, _impl_.data_),
};

static const ::_pbi::MigrationSchema
    schemas[] ABSL_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::binmessage::BinaryMessage)},
};
static const ::_pb::Message* const file_default_instances[] = {
    &::binmessage::_BinaryMessage_default_instance_._instance,
};
const char descriptor_table_protodef_model_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\013model.proto\022\nbinmessage\"p\n\rBinaryMessa"
    "ge\022\022\n\ncompressed\030\001 \001(\010\022\014\n\004code\030\002 \001(\005\022\021\n\t"
    "recepient\030\003 \001(\t\022\016\n\006sender\030\004 \001(\t\022\014\n\004meta\030"
    "\005 \001(\t\022\014\n\004data\030\006 \001(\014b\006proto3"
};
static ::absl::once_flag descriptor_table_model_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_model_2eproto = {
    false,
    false,
    147,
    descriptor_table_protodef_model_2eproto,
    "model.proto",
    &descriptor_table_model_2eproto_once,
    nullptr,
    0,
    1,
    schemas,
    file_default_instances,
    TableStruct_model_2eproto::offsets,
    file_level_enum_descriptors_model_2eproto,
    file_level_service_descriptors_model_2eproto,
};
namespace binmessage {
// ===================================================================

class BinaryMessage::_Internal {
 public:
};

BinaryMessage::BinaryMessage(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:binmessage.BinaryMessage)
}
inline PROTOBUF_NDEBUG_INLINE BinaryMessage::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::binmessage::BinaryMessage& from_msg)
      : recepient_(arena, from.recepient_),
        sender_(arena, from.sender_),
        meta_(arena, from.meta_),
        data_(arena, from.data_),
        _cached_size_{0} {}

BinaryMessage::BinaryMessage(
    ::google::protobuf::Arena* arena,
    const BinaryMessage& from)
    : ::google::protobuf::Message(arena) {
  BinaryMessage* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);
  ::memcpy(reinterpret_cast<char *>(&_impl_) +
               offsetof(Impl_, compressed_),
           reinterpret_cast<const char *>(&from._impl_) +
               offsetof(Impl_, compressed_),
           offsetof(Impl_, code_) -
               offsetof(Impl_, compressed_) +
               sizeof(Impl_::code_));

  // @@protoc_insertion_point(copy_constructor:binmessage.BinaryMessage)
}
inline PROTOBUF_NDEBUG_INLINE BinaryMessage::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : recepient_(arena),
        sender_(arena),
        meta_(arena),
        data_(arena),
        _cached_size_{0} {}

inline void BinaryMessage::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  ::memset(reinterpret_cast<char *>(&_impl_) +
               offsetof(Impl_, compressed_),
           0,
           offsetof(Impl_, code_) -
               offsetof(Impl_, compressed_) +
               sizeof(Impl_::code_));
}
BinaryMessage::~BinaryMessage() {
  // @@protoc_insertion_point(destructor:binmessage.BinaryMessage)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void BinaryMessage::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.recepient_.Destroy();
  _impl_.sender_.Destroy();
  _impl_.meta_.Destroy();
  _impl_.data_.Destroy();
  _impl_.~Impl_();
}

const ::google::protobuf::MessageLite::ClassData*
BinaryMessage::GetClassData() const {
  PROTOBUF_CONSTINIT static const ::google::protobuf::MessageLite::
      ClassDataFull _data_ = {
          {
              &_table_.header,
              nullptr,  // OnDemandRegisterArenaDtor
              nullptr,  // IsInitialized
              &BinaryMessage::MergeImpl,
              PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_._cached_size_),
              false,
          },
          &BinaryMessage::kDescriptorMethods,
          &descriptor_table_model_2eproto,
          nullptr,  // tracker
      };
  ::google::protobuf::internal::PrefetchToLocalCache(&_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_data_.tc_table);
  return _data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<3, 6, 0, 52, 2> BinaryMessage::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    6, 56,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967232,  // skipmap
    offsetof(decltype(_table_), field_entries),
    6,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_BinaryMessage_default_instance_._instance,
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::binmessage::BinaryMessage>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    {::_pbi::TcParser::MiniParse, {}},
    // bool compressed = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<bool, offsetof(BinaryMessage, _impl_.compressed_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.compressed_)}},
    // int32 code = 2;
    {::_pbi::TcParser::SingularVarintNoZag1<::uint32_t, offsetof(BinaryMessage, _impl_.code_), 63>(),
     {16, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.code_)}},
    // string recepient = 3;
    {::_pbi::TcParser::FastUS1,
     {26, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.recepient_)}},
    // string sender = 4;
    {::_pbi::TcParser::FastUS1,
     {34, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.sender_)}},
    // string meta = 5;
    {::_pbi::TcParser::FastUS1,
     {42, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.meta_)}},
    // bytes data = 6;
    {::_pbi::TcParser::FastBS1,
     {50, 63, 0, PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.data_)}},
    {::_pbi::TcParser::MiniParse, {}},
  }}, {{
    65535, 65535
  }}, {{
    // bool compressed = 1;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.compressed_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBool)},
    // int32 code = 2;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.code_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kInt32)},
    // string recepient = 3;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.recepient_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
    // string sender = 4;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.sender_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
    // string meta = 5;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.meta_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
    // bytes data = 6;
    {PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.data_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
    "\30\0\0\11\6\4\0\0"
    "binmessage.BinaryMessage"
    "recepient"
    "sender"
    "meta"
  }},
};

PROTOBUF_NOINLINE void BinaryMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:binmessage.BinaryMessage)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.recepient_.ClearToEmpty();
  _impl_.sender_.ClearToEmpty();
  _impl_.meta_.ClearToEmpty();
  _impl_.data_.ClearToEmpty();
  ::memset(&_impl_.compressed_, 0, static_cast<::size_t>(
      reinterpret_cast<char*>(&_impl_.code_) -
      reinterpret_cast<char*>(&_impl_.compressed_)) + sizeof(_impl_.code_));
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

::uint8_t* BinaryMessage::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:binmessage.BinaryMessage)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // bool compressed = 1;
  if (this->_internal_compressed() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteBoolToArray(
        1, this->_internal_compressed(), target);
  }

  // int32 code = 2;
  if (this->_internal_code() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::
        WriteInt32ToArrayWithField<2>(
            stream, this->_internal_code(), target);
  }

  // string recepient = 3;
  if (!this->_internal_recepient().empty()) {
    const std::string& _s = this->_internal_recepient();
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "binmessage.BinaryMessage.recepient");
    target = stream->WriteStringMaybeAliased(3, _s, target);
  }

  // string sender = 4;
  if (!this->_internal_sender().empty()) {
    const std::string& _s = this->_internal_sender();
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "binmessage.BinaryMessage.sender");
    target = stream->WriteStringMaybeAliased(4, _s, target);
  }

  // string meta = 5;
  if (!this->_internal_meta().empty()) {
    const std::string& _s = this->_internal_meta();
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "binmessage.BinaryMessage.meta");
    target = stream->WriteStringMaybeAliased(5, _s, target);
  }

  // bytes data = 6;
  if (!this->_internal_data().empty()) {
    const std::string& _s = this->_internal_data();
    target = stream->WriteBytesMaybeAliased(6, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:binmessage.BinaryMessage)
  return target;
}

::size_t BinaryMessage::ByteSizeLong() const {
  // @@protoc_insertion_point(message_byte_size_start:binmessage.BinaryMessage)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void)cached_has_bits;

  ::_pbi::Prefetch5LinesFrom7Lines(
      reinterpret_cast<const void*>(this));
   {
    // string recepient = 3;
    if (!this->_internal_recepient().empty()) {
      total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                      this->_internal_recepient());
    }
    // string sender = 4;
    if (!this->_internal_sender().empty()) {
      total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                      this->_internal_sender());
    }
    // string meta = 5;
    if (!this->_internal_meta().empty()) {
      total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                      this->_internal_meta());
    }
    // bytes data = 6;
    if (!this->_internal_data().empty()) {
      total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                      this->_internal_data());
    }
    // bool compressed = 1;
    if (this->_internal_compressed() != 0) {
      total_size += 2;
    }
    // int32 code = 2;
    if (this->_internal_code() != 0) {
      total_size += ::_pbi::WireFormatLite::Int32SizePlusOne(
          this->_internal_code());
    }
  }
  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

void BinaryMessage::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<BinaryMessage*>(&to_msg);
  auto& from = static_cast<const BinaryMessage&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:binmessage.BinaryMessage)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_recepient().empty()) {
    _this->_internal_set_recepient(from._internal_recepient());
  }
  if (!from._internal_sender().empty()) {
    _this->_internal_set_sender(from._internal_sender());
  }
  if (!from._internal_meta().empty()) {
    _this->_internal_set_meta(from._internal_meta());
  }
  if (!from._internal_data().empty()) {
    _this->_internal_set_data(from._internal_data());
  }
  if (from._internal_compressed() != 0) {
    _this->_impl_.compressed_ = from._impl_.compressed_;
  }
  if (from._internal_code() != 0) {
    _this->_impl_.code_ = from._impl_.code_;
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void BinaryMessage::CopyFrom(const BinaryMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:binmessage.BinaryMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void BinaryMessage::InternalSwap(BinaryMessage* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.recepient_, &other->_impl_.recepient_, arena);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.sender_, &other->_impl_.sender_, arena);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.meta_, &other->_impl_.meta_, arena);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.data_, &other->_impl_.data_, arena);
  ::google::protobuf::internal::memswap<
      PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.code_)
      + sizeof(BinaryMessage::_impl_.code_)
      - PROTOBUF_FIELD_OFFSET(BinaryMessage, _impl_.compressed_)>(
          reinterpret_cast<char*>(&_impl_.compressed_),
          reinterpret_cast<char*>(&other->_impl_.compressed_));
}

::google::protobuf::Metadata BinaryMessage::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace binmessage
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::std::false_type
    _static_init2_ PROTOBUF_UNUSED =
        (::_pbi::AddDescriptors(&descriptor_table_model_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
