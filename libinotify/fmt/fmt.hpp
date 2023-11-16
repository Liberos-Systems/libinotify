#include <fmt/format.h>
#include <filesystem>

namespace fmt
{
    template <>
    struct formatter<std::filesystem::path>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) -> decltype(ctx.begin())
        {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const std::filesystem::path &p, FormatContext &ctx) -> decltype(format_to(ctx.out(), "{}", p.string()))
        {
            return format_to(ctx.out(), "{}", p.string());
        }
    };
}
