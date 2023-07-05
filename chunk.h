#include <iterator>
template<typename Iterator, typename Func, typename Distance>
void chunks(Iterator begin, Iterator end, Distance k ,Func f){
    Iterator chunk_begin;
    Iterator chunk_end;
    chunk_end = chunk_begin = begin;

    do{
        if(std::distance(chunk_end, end) < k)
            chunk_end = end;
        else
            std::advance(chunk_end, k);
        f(chunk_begin,chunk_end);
        chunk_begin = chunk_end;
    }while(std::distance(chunk_begin,end) > 0);
}