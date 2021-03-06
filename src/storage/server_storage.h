#ifndef SERVER_STORAGE_H_Q4KYZGRA
#define SERVER_STORAGE_H_Q4KYZGRA

#include "util/storage/dense_storage.h"

#include "lib.h"

namespace woops
{
template<typename T>
class ServerStorage: public DenseStorage<T>
{
public:
    ServerStorage (Tableid id) {
        auto&& partition = Lib::Placement().GetPartitions(id).at(Lib::ThisHost());
        this->data_.resize(partition.end - partition.begin);
    }

    Bytes Encode() override {
        std::lock_guard<std::mutex> lock(this->mu_);
        Bytes ret = Bytes{(Byte*)this->data_.data(), this->data_.size() * sizeof(T)};
        this->zerofy();
        return ret;
    }

    void Decode(
            MAYBE_UNUSED Hostid from,
            MAYBE_UNUSED Hostid to,
            const Bytes& bytes) override {
        const T* data = reinterpret_cast<const T*>(bytes.data());
        size_t size = bytes.size() / sizeof(T);
        std::lock_guard<std::mutex> lock(this->mu_);
        this->update(data, size);
    }
};
 
} /* woops */ 



#endif /* end of include guard: SERVER_STORAGE_H_Q4KYZGRA */
