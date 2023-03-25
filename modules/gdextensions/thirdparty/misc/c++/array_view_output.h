#if !defined NOSTD_ARRAY_VIEW_OUTPUT_H_INCLUDED
#define NOSTD_ARRAY_VIEW_OUTPUT_H_INCLUDED

#include <ostream>

namespace nostd {
template<class T>
class array_view;
} // namespace nostd

template<class T>
std::ostream &operator<<(std::ostream &ost, nostd::array_view<T> const& av)
{
    ost << '{';

    if(!av.empty()) {
        auto itr = std::begin(av);
        auto const end = std::end(av);
        while (true) {
            ost << *itr;
            if (++itr != end) {
                ost << ", ";
            } else {
                break;
            }
        }
    }

    ost << '}';
    return ost;
}

#endif // ARV_ARRAY_VIEW_OUTPUT_H_INCLUDED
