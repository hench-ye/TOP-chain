// This file is generated by rust-protobuf 2.27.1. Do not edit
// @generated

// https://github.com/rust-lang/rust-clippy/issues/702
#![allow(unknown_lints)]
#![allow(clippy::all)]

#![allow(unused_attributes)]
#![cfg_attr(rustfmt, rustfmt::skip)]

#![allow(box_pointers)]
#![allow(dead_code)]
#![allow(missing_docs)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(trivial_casts)]
#![allow(unused_imports)]
#![allow(unused_results)]
//! Generated file from `proto_basic.proto`

/// Generated files are compatible only with the same version
/// of protobuf runtime.
// const _PROTOBUF_VERSION_CHECK: () = ::protobuf::VERSION_2_27_1;

#[derive(PartialEq,Clone,Default)]
pub struct ProtoAddress {
    // message fields
    pub value: ::std::vec::Vec<u8>,
    // special fields
    pub unknown_fields: ::protobuf::UnknownFields,
    pub cached_size: ::protobuf::CachedSize,
}

impl<'a> ::std::default::Default for &'a ProtoAddress {
    fn default() -> &'a ProtoAddress {
        <ProtoAddress as ::protobuf::Message>::default_instance()
    }
}

impl ProtoAddress {
    pub fn new() -> ProtoAddress {
        ::std::default::Default::default()
    }

    // bytes value = 1;


    pub fn get_value(&self) -> &[u8] {
        &self.value
    }
    pub fn clear_value(&mut self) {
        self.value.clear();
    }

    // Param is passed by value, moved
    pub fn set_value(&mut self, v: ::std::vec::Vec<u8>) {
        self.value = v;
    }

    // Mutable pointer to the field.
    // If field is not initialized, it is initialized with default value first.
    pub fn mut_value(&mut self) -> &mut ::std::vec::Vec<u8> {
        &mut self.value
    }

    // Take field
    pub fn take_value(&mut self) -> ::std::vec::Vec<u8> {
        ::std::mem::replace(&mut self.value, ::std::vec::Vec::new())
    }
}

impl ::protobuf::Message for ProtoAddress {
    fn is_initialized(&self) -> bool {
        true
    }

    fn merge_from(&mut self, is: &mut ::protobuf::CodedInputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        while !is.eof()? {
            let (field_number, wire_type) = is.read_tag_unpack()?;
            match field_number {
                1 => {
                    ::protobuf::rt::read_singular_proto3_bytes_into(wire_type, is, &mut self.value)?;
                },
                _ => {
                    ::protobuf::rt::read_unknown_or_skip_group(field_number, wire_type, is, self.mut_unknown_fields())?;
                },
            };
        }
        ::std::result::Result::Ok(())
    }

    // Compute sizes of nested messages
    #[allow(unused_variables)]
    fn compute_size(&self) -> u32 {
        let mut my_size = 0;
        if !self.value.is_empty() {
            my_size += ::protobuf::rt::bytes_size(1, &self.value);
        }
        my_size += ::protobuf::rt::unknown_fields_size(self.get_unknown_fields());
        self.cached_size.set(my_size);
        my_size
    }

    fn write_to_with_cached_sizes(&self, os: &mut ::protobuf::CodedOutputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        if !self.value.is_empty() {
            os.write_bytes(1, &self.value)?;
        }
        os.write_unknown_fields(self.get_unknown_fields())?;
        ::std::result::Result::Ok(())
    }

    fn get_cached_size(&self) -> u32 {
        self.cached_size.get()
    }

    fn get_unknown_fields(&self) -> &::protobuf::UnknownFields {
        &self.unknown_fields
    }

    fn mut_unknown_fields(&mut self) -> &mut ::protobuf::UnknownFields {
        &mut self.unknown_fields
    }

    fn as_any(&self) -> &dyn (::std::any::Any) {
        self as &dyn (::std::any::Any)
    }
    fn as_any_mut(&mut self) -> &mut dyn (::std::any::Any) {
        self as &mut dyn (::std::any::Any)
    }
    fn into_any(self: ::std::boxed::Box<Self>) -> ::std::boxed::Box<dyn (::std::any::Any)> {
        self
    }

    fn descriptor(&self) -> &'static ::protobuf::reflect::MessageDescriptor {
        Self::descriptor_static()
    }

    fn new() -> ProtoAddress {
        ProtoAddress::new()
    }

    fn descriptor_static() -> &'static ::protobuf::reflect::MessageDescriptor {
        static descriptor: ::protobuf::rt::LazyV2<::protobuf::reflect::MessageDescriptor> = ::protobuf::rt::LazyV2::INIT;
        descriptor.get(|| {
            let mut fields = ::std::vec::Vec::new();
            fields.push(::protobuf::reflect::accessor::make_simple_field_accessor::<_, ::protobuf::types::ProtobufTypeBytes>(
                "value",
                |m: &ProtoAddress| { &m.value },
                |m: &mut ProtoAddress| { &mut m.value },
            ));
            ::protobuf::reflect::MessageDescriptor::new_pb_name::<ProtoAddress>(
                "ProtoAddress",
                fields,
                file_descriptor_proto()
            )
        })
    }

    fn default_instance() -> &'static ProtoAddress {
        static instance: ::protobuf::rt::LazyV2<ProtoAddress> = ::protobuf::rt::LazyV2::INIT;
        instance.get(ProtoAddress::new)
    }
}

impl ::protobuf::Clear for ProtoAddress {
    fn clear(&mut self) {
        self.value.clear();
        self.unknown_fields.clear();
    }
}

impl ::std::fmt::Debug for ProtoAddress {
    fn fmt(&self, f: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
        ::protobuf::text_format::fmt(self, f)
    }
}

impl ::protobuf::reflect::ProtobufValue for ProtoAddress {
    fn as_ref(&self) -> ::protobuf::reflect::ReflectValueRef {
        ::protobuf::reflect::ReflectValueRef::Message(self)
    }
}

#[derive(PartialEq,Clone,Default)]
pub struct RawU256 {
    // message fields
    pub data: ::std::vec::Vec<u8>,
    // special fields
    pub unknown_fields: ::protobuf::UnknownFields,
    pub cached_size: ::protobuf::CachedSize,
}

impl<'a> ::std::default::Default for &'a RawU256 {
    fn default() -> &'a RawU256 {
        <RawU256 as ::protobuf::Message>::default_instance()
    }
}

impl RawU256 {
    pub fn new() -> RawU256 {
        ::std::default::Default::default()
    }

    // bytes data = 1;


    pub fn get_data(&self) -> &[u8] {
        &self.data
    }
    pub fn clear_data(&mut self) {
        self.data.clear();
    }

    // Param is passed by value, moved
    pub fn set_data(&mut self, v: ::std::vec::Vec<u8>) {
        self.data = v;
    }

    // Mutable pointer to the field.
    // If field is not initialized, it is initialized with default value first.
    pub fn mut_data(&mut self) -> &mut ::std::vec::Vec<u8> {
        &mut self.data
    }

    // Take field
    pub fn take_data(&mut self) -> ::std::vec::Vec<u8> {
        ::std::mem::replace(&mut self.data, ::std::vec::Vec::new())
    }
}

impl ::protobuf::Message for RawU256 {
    fn is_initialized(&self) -> bool {
        true
    }

    fn merge_from(&mut self, is: &mut ::protobuf::CodedInputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        while !is.eof()? {
            let (field_number, wire_type) = is.read_tag_unpack()?;
            match field_number {
                1 => {
                    ::protobuf::rt::read_singular_proto3_bytes_into(wire_type, is, &mut self.data)?;
                },
                _ => {
                    ::protobuf::rt::read_unknown_or_skip_group(field_number, wire_type, is, self.mut_unknown_fields())?;
                },
            };
        }
        ::std::result::Result::Ok(())
    }

    // Compute sizes of nested messages
    #[allow(unused_variables)]
    fn compute_size(&self) -> u32 {
        let mut my_size = 0;
        if !self.data.is_empty() {
            my_size += ::protobuf::rt::bytes_size(1, &self.data);
        }
        my_size += ::protobuf::rt::unknown_fields_size(self.get_unknown_fields());
        self.cached_size.set(my_size);
        my_size
    }

    fn write_to_with_cached_sizes(&self, os: &mut ::protobuf::CodedOutputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        if !self.data.is_empty() {
            os.write_bytes(1, &self.data)?;
        }
        os.write_unknown_fields(self.get_unknown_fields())?;
        ::std::result::Result::Ok(())
    }

    fn get_cached_size(&self) -> u32 {
        self.cached_size.get()
    }

    fn get_unknown_fields(&self) -> &::protobuf::UnknownFields {
        &self.unknown_fields
    }

    fn mut_unknown_fields(&mut self) -> &mut ::protobuf::UnknownFields {
        &mut self.unknown_fields
    }

    fn as_any(&self) -> &dyn (::std::any::Any) {
        self as &dyn (::std::any::Any)
    }
    fn as_any_mut(&mut self) -> &mut dyn (::std::any::Any) {
        self as &mut dyn (::std::any::Any)
    }
    fn into_any(self: ::std::boxed::Box<Self>) -> ::std::boxed::Box<dyn (::std::any::Any)> {
        self
    }

    fn descriptor(&self) -> &'static ::protobuf::reflect::MessageDescriptor {
        Self::descriptor_static()
    }

    fn new() -> RawU256 {
        RawU256::new()
    }

    fn descriptor_static() -> &'static ::protobuf::reflect::MessageDescriptor {
        static descriptor: ::protobuf::rt::LazyV2<::protobuf::reflect::MessageDescriptor> = ::protobuf::rt::LazyV2::INIT;
        descriptor.get(|| {
            let mut fields = ::std::vec::Vec::new();
            fields.push(::protobuf::reflect::accessor::make_simple_field_accessor::<_, ::protobuf::types::ProtobufTypeBytes>(
                "data",
                |m: &RawU256| { &m.data },
                |m: &mut RawU256| { &mut m.data },
            ));
            ::protobuf::reflect::MessageDescriptor::new_pb_name::<RawU256>(
                "RawU256",
                fields,
                file_descriptor_proto()
            )
        })
    }

    fn default_instance() -> &'static RawU256 {
        static instance: ::protobuf::rt::LazyV2<RawU256> = ::protobuf::rt::LazyV2::INIT;
        instance.get(RawU256::new)
    }
}

impl ::protobuf::Clear for RawU256 {
    fn clear(&mut self) {
        self.data.clear();
        self.unknown_fields.clear();
    }
}

impl ::std::fmt::Debug for RawU256 {
    fn fmt(&self, f: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
        ::protobuf::text_format::fmt(self, f)
    }
}

impl ::protobuf::reflect::ProtobufValue for RawU256 {
    fn as_ref(&self) -> ::protobuf::reflect::ReflectValueRef {
        ::protobuf::reflect::ReflectValueRef::Message(self)
    }
}

#[derive(PartialEq,Clone,Default)]
pub struct WeiU256 {
    // message fields
    pub data: ::std::vec::Vec<u8>,
    // special fields
    pub unknown_fields: ::protobuf::UnknownFields,
    pub cached_size: ::protobuf::CachedSize,
}

impl<'a> ::std::default::Default for &'a WeiU256 {
    fn default() -> &'a WeiU256 {
        <WeiU256 as ::protobuf::Message>::default_instance()
    }
}

impl WeiU256 {
    pub fn new() -> WeiU256 {
        ::std::default::Default::default()
    }

    // bytes data = 1;


    pub fn get_data(&self) -> &[u8] {
        &self.data
    }
    pub fn clear_data(&mut self) {
        self.data.clear();
    }

    // Param is passed by value, moved
    pub fn set_data(&mut self, v: ::std::vec::Vec<u8>) {
        self.data = v;
    }

    // Mutable pointer to the field.
    // If field is not initialized, it is initialized with default value first.
    pub fn mut_data(&mut self) -> &mut ::std::vec::Vec<u8> {
        &mut self.data
    }

    // Take field
    pub fn take_data(&mut self) -> ::std::vec::Vec<u8> {
        ::std::mem::replace(&mut self.data, ::std::vec::Vec::new())
    }
}

impl ::protobuf::Message for WeiU256 {
    fn is_initialized(&self) -> bool {
        true
    }

    fn merge_from(&mut self, is: &mut ::protobuf::CodedInputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        while !is.eof()? {
            let (field_number, wire_type) = is.read_tag_unpack()?;
            match field_number {
                1 => {
                    ::protobuf::rt::read_singular_proto3_bytes_into(wire_type, is, &mut self.data)?;
                },
                _ => {
                    ::protobuf::rt::read_unknown_or_skip_group(field_number, wire_type, is, self.mut_unknown_fields())?;
                },
            };
        }
        ::std::result::Result::Ok(())
    }

    // Compute sizes of nested messages
    #[allow(unused_variables)]
    fn compute_size(&self) -> u32 {
        let mut my_size = 0;
        if !self.data.is_empty() {
            my_size += ::protobuf::rt::bytes_size(1, &self.data);
        }
        my_size += ::protobuf::rt::unknown_fields_size(self.get_unknown_fields());
        self.cached_size.set(my_size);
        my_size
    }

    fn write_to_with_cached_sizes(&self, os: &mut ::protobuf::CodedOutputStream<'_>) -> ::protobuf::ProtobufResult<()> {
        if !self.data.is_empty() {
            os.write_bytes(1, &self.data)?;
        }
        os.write_unknown_fields(self.get_unknown_fields())?;
        ::std::result::Result::Ok(())
    }

    fn get_cached_size(&self) -> u32 {
        self.cached_size.get()
    }

    fn get_unknown_fields(&self) -> &::protobuf::UnknownFields {
        &self.unknown_fields
    }

    fn mut_unknown_fields(&mut self) -> &mut ::protobuf::UnknownFields {
        &mut self.unknown_fields
    }

    fn as_any(&self) -> &dyn (::std::any::Any) {
        self as &dyn (::std::any::Any)
    }
    fn as_any_mut(&mut self) -> &mut dyn (::std::any::Any) {
        self as &mut dyn (::std::any::Any)
    }
    fn into_any(self: ::std::boxed::Box<Self>) -> ::std::boxed::Box<dyn (::std::any::Any)> {
        self
    }

    fn descriptor(&self) -> &'static ::protobuf::reflect::MessageDescriptor {
        Self::descriptor_static()
    }

    fn new() -> WeiU256 {
        WeiU256::new()
    }

    fn descriptor_static() -> &'static ::protobuf::reflect::MessageDescriptor {
        static descriptor: ::protobuf::rt::LazyV2<::protobuf::reflect::MessageDescriptor> = ::protobuf::rt::LazyV2::INIT;
        descriptor.get(|| {
            let mut fields = ::std::vec::Vec::new();
            fields.push(::protobuf::reflect::accessor::make_simple_field_accessor::<_, ::protobuf::types::ProtobufTypeBytes>(
                "data",
                |m: &WeiU256| { &m.data },
                |m: &mut WeiU256| { &mut m.data },
            ));
            ::protobuf::reflect::MessageDescriptor::new_pb_name::<WeiU256>(
                "WeiU256",
                fields,
                file_descriptor_proto()
            )
        })
    }

    fn default_instance() -> &'static WeiU256 {
        static instance: ::protobuf::rt::LazyV2<WeiU256> = ::protobuf::rt::LazyV2::INIT;
        instance.get(WeiU256::new)
    }
}

impl ::protobuf::Clear for WeiU256 {
    fn clear(&mut self) {
        self.data.clear();
        self.unknown_fields.clear();
    }
}

impl ::std::fmt::Debug for WeiU256 {
    fn fmt(&self, f: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
        ::protobuf::text_format::fmt(self, f)
    }
}

impl ::protobuf::reflect::ProtobufValue for WeiU256 {
    fn as_ref(&self) -> ::protobuf::reflect::ReflectValueRef {
        ::protobuf::reflect::ReflectValueRef::Message(self)
    }
}

static file_descriptor_proto_data: &'static [u8] = b"\
    \n\x11proto_basic.proto\x12\x14top.evm_engine.basic\"$\n\x0cProtoAddress\
    \x12\x14\n\x05value\x18\x01\x20\x01(\x0cR\x05value\"\x1d\n\x07RawU256\
    \x12\x12\n\x04data\x18\x01\x20\x01(\x0cR\x04data\"\x1d\n\x07WeiU256\x12\
    \x12\n\x04data\x18\x01\x20\x01(\x0cR\x04dataJ\xa3\x03\n\x06\x12\x04\0\0\
    \x0e\x01\n\x08\n\x01\x0c\x12\x03\0\0\x12\n\x08\n\x01\x02\x12\x03\x02\x08\
    \x1c\n\n\n\x02\x04\0\x12\x04\x04\0\x06\x01\n\n\n\x03\x04\0\x01\x12\x03\
    \x04\x08\x14\n8\n\x04\x04\0\x02\0\x12\x03\x05\x04\x14\"+\x20\x20use\x20b\
    ytes\x20to\x20represent\x20H160\x20(\x20[u8;\x2020]\x20)\n\n\r\n\x05\x04\
    \0\x02\0\x04\x12\x04\x05\x04\x04\x16\n\x0c\n\x05\x04\0\x02\0\x05\x12\x03\
    \x05\x04\t\n\x0c\n\x05\x04\0\x02\0\x01\x12\x03\x05\n\x0f\n\x0c\n\x05\x04\
    \0\x02\0\x03\x12\x03\x05\x12\x13\n\n\n\x02\x04\x01\x12\x04\x08\0\n\x01\n\
    \n\n\x03\x04\x01\x01\x12\x03\x08\x08\x0f\n+\n\x04\x04\x01\x02\0\x12\x03\
    \t\x04\x13\"\x1e\x20pub\x20type\x20RawU256\x20=\x20[u8;\x2032];\n\n\r\n\
    \x05\x04\x01\x02\0\x04\x12\x04\t\x04\x08\x11\n\x0c\n\x05\x04\x01\x02\0\
    \x05\x12\x03\t\x04\t\n\x0c\n\x05\x04\x01\x02\0\x01\x12\x03\t\n\x0e\n\x0c\
    \n\x05\x04\x01\x02\0\x03\x12\x03\t\x11\x12\n\n\n\x02\x04\x02\x12\x04\x0c\
    \0\x0e\x01\n\n\n\x03\x04\x02\x01\x12\x03\x0c\x08\x0f\n+\n\x04\x04\x02\
    \x02\0\x12\x03\r\x04\x13\"\x1e\x20pub\x20type\x20WeiU256\x20=\x20[u8;\
    \x2032];\n\n\r\n\x05\x04\x02\x02\0\x04\x12\x04\r\x04\x0c\x11\n\x0c\n\x05\
    \x04\x02\x02\0\x05\x12\x03\r\x04\t\n\x0c\n\x05\x04\x02\x02\0\x01\x12\x03\
    \r\n\x0e\n\x0c\n\x05\x04\x02\x02\0\x03\x12\x03\r\x11\x12b\x06proto3\
";

static file_descriptor_proto_lazy: ::protobuf::rt::LazyV2<::protobuf::descriptor::FileDescriptorProto> = ::protobuf::rt::LazyV2::INIT;

fn parse_descriptor_proto() -> ::protobuf::descriptor::FileDescriptorProto {
    ::protobuf::Message::parse_from_bytes(file_descriptor_proto_data).unwrap()
}

pub fn file_descriptor_proto() -> &'static ::protobuf::descriptor::FileDescriptorProto {
    file_descriptor_proto_lazy.get(|| {
        parse_descriptor_proto()
    })
}
