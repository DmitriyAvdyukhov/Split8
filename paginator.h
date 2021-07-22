#pragma once
#include <vector>
#include <iostream>

template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(const Iterator& it_begin, const Iterator it_end)
        : it_begin_(it_begin)
        , it_end_(it_end)
        , size_(std::distance(it_begin_, it_end_))
    {}

    Iterator begin() const
    {
        return it_begin_;
    }

    Iterator end() const
    {
        return it_end_;
    }

    size_t size() const
    {
        return std::distance(it_begin_, it_end_);
    }

private:
    const Iterator it_begin_, it_end_;
    size_t   size_;
};

template <typename Iterator>
class Paginator
{
public:

    Paginator(Iterator it_bigin, Iterator it_end, size_t page_size)
    {
        for (size_t left = std::distance(it_bigin, it_end); left > 0;)
        {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = std::next(it_bigin, current_page_size);
            it_range_.push_back({ it_bigin, current_page_end });

            left -= current_page_size;
            it_bigin = current_page_end;
        }
    }

    auto begin() const
    {
        return  it_range_.begin();
    }

    auto end() const
    {
        return  it_range_.end();
    }

    size_t size() const
    {
        return it_range_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> it_range_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size)
{
    return Paginator(begin(c), end(c), page_size);
}


template <typename Iterator>
std::ostream& operator <<(std::ostream& out, const IteratorRange<Iterator>& range)
{
    for (auto it = range.begin(); it != range.end(); ++it)
    {
        out << *it;
    }
    return out;
}
