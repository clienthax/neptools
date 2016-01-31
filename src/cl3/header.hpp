#ifndef CL3_HEADER_HPP
#define CL3_HEADER_HPP
#pragma once

#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Cl3
{

struct Header
{
    char magic[4];
    boost::endian::little_uint32_t field_04;
    boost::endian::little_uint32_t field_08;
    boost::endian::little_uint32_t num_sections;
    boost::endian::little_uint32_t secs_offset;
    boost::endian::little_uint32_t field_14;

    bool IsValid(size_t file_size) const noexcept;
};
static_assert(sizeof(Header) == 0x18, "");

class HeaderItem final : public Item
{
public:
    HeaderItem(Key k, Context* ctx, const Header* hdr);
    static HeaderItem* CreateAndInsert(RawItem* ritem);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override { return sizeof(Header); }

    uint32_t num_sections;
    const Label* sections;
};

}

#endif