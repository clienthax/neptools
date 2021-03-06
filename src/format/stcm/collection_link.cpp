#include "collection_link.hpp"
#include "../context.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

namespace Neptools
{
namespace Stcm
{

void CollectionLinkHeaderItem::Header::Validate(FilePosition file_size) const
{
#define VALIDATE(x) \
    NEPTOOLS_VALIDATE_FIELD("Stcm::CollectionLinkHeaderItem::Header", x)

    VALIDATE(field_00 == 0);
    VALIDATE(offset <= file_size);
    VALIDATE(offset + sizeof(CollectionLinkItem::Entry)*count <= file_size);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
    VALIDATE(field_20 == 0 && field_24 == 0 && field_28 == 0 && field_2c == 0);
    VALIDATE(field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0);
#undef VALIDATE
}

void CollectionLinkItem::Entry::Validate(FilePosition file_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm::CollectionLinkItem::Entry", x)
    VALIDATE(name_0 <= file_size);
    VALIDATE(name_1 <= file_size);
    VALIDATE(ptr == 0);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
#undef VALIDATE
}

CollectionLinkHeaderItem::CollectionLinkHeaderItem(
    Key k, Context* ctx, const Header& s)
    : Item{k, ctx}
{
    s.Validate(GetUnsafeContext().GetSize());

    data = &GetUnsafeContext().CreateLabelFallback("collection_link", s.offset);
}

CollectionLinkHeaderItem& CollectionLinkHeaderItem::CreateAndInsert(
    ItemPointer ptr)
{
    auto x = RawItem::Get<Header>(ptr);
    auto& ret = x.ritem.SplitCreate<CollectionLinkHeaderItem>(ptr.offset, x.t);

    auto ptr2 = ret.data->ptr;
    auto* ritem2 = ptr2.Maybe<RawItem>();
    if (!ritem2)
    {
        // HACK!
        NEPTOOLS_VALIDATE_FIELD(
            "Stcm::CollectionLinkHeaderItem",
            ptr2.offset == 0 && x.t.count == 0);
        auto& eof = ptr2.AsChecked0<EofItem>();
        eof.Replace(eof.GetUnsafeContext().Create<CollectionLinkItem>());
        return ret;
    }

    auto e = RawItem::GetSource(ptr2, x.t.count*sizeof(CollectionLinkItem::Entry));

    e.ritem.SplitCreate<CollectionLinkItem>(ptr2.offset, e.src, x.t.count);

    return ret;
}

void CollectionLinkHeaderItem::Dump_(Sink& sink) const
{
    Header hdr{};
    hdr.offset = ToFilePos(data->ptr);
    hdr.count = data->ptr.As0<CollectionLinkItem>().entries.size();
    sink.WriteGen(hdr);
}

void CollectionLinkHeaderItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);
    os << "collection_link_header(@" << data->name << ")";
}

CollectionLinkItem::CollectionLinkItem(
    Key k, Context* ctx, Source src, uint32_t count)
    : Item{k, ctx}
{
    AddInfo(&CollectionLinkItem::Parse_, ADD_SOURCE(src), this, src, count);
}

void CollectionLinkItem::Dispose() noexcept
{
    entries.clear();
    Item::Dispose();
}

void CollectionLinkItem::Parse_(Source& src, uint32_t count)
{
    auto& ctx = GetUnsafeContext();
    entries.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto e = src.ReadGen<Entry>();
        e.Validate(ctx.GetSize());
        entries.push_back({
            &ctx.GetLabelTo(e.name_0),
            &ctx.GetLabelTo(e.name_1)});
    }
}

void CollectionLinkItem::Dump_(Sink& sink) const
{
    Entry ee{};
    for (const auto& e : entries)
    {
        ee.name_0 = ToFilePos(e.name_0->ptr);
        ee.name_1 = ToFilePos(e.name_1->ptr);
        sink.WriteGen(ee);
    }
}

void CollectionLinkItem::Inspect_(std::ostream& os) const
{
    bool good_labels = true;
    for (auto& lbl : GetLabels())
        if (lbl.ptr.offset % sizeof(Entry) != 0 ||
            lbl.ptr.offset / sizeof(Entry) >= entries.size())
        {
            good_labels = false;
            break;
        }
    if (!good_labels)
        Item::Inspect_(os);

    size_t i = 0;
    for (const auto& e : entries)
    {
        if (good_labels)
        {
            auto x = GetLabels().equal_range(i);
            for (auto it = x.first; it != x.second; ++it)
                os << '@' << it->name << ":\n";
            i += sizeof(Entry);
        }


        os << "collection_link(@" << e.name_0->name << ", @" << e.name_1->name
           << ")\n";
    }
}

}
}
