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
        offset_ = partition.begin;
    }

    Bytes Encode() override {
        std::lock_guard<std::mutex> lock(this->mu_);
        Bytes ret;
        for (ParamIndex i = 0; i < (ParamIndex)this->data_.size(); ++i) {
            const T& val = this->data_[i];
            if (!val) continue;
            ParamIndex idx = i + offset_;
            ret.append((Byte*)&(idx), (Byte*)(&(idx) + 1));
            ret.append((Byte*)&(val), (Byte*)(&(val) + 1));
        }
        this->zerofy();
        return ret;
    }
    void Decode(
            MAYBE_UNUSED Hostid form,
            MAYBE_UNUSED Hostid to,
            const Bytes& bytes) {
        std::lock_guard<std::mutex> lock(this->mu_);
        auto it = bytes.begin();
        while (it != bytes.end()) {
            ParamIndex idx = *reinterpret_cast<const ParamIndex*>(&(*it));
            std::advance(it, sizeof(ParamIndex)/sizeof(Byte));
            this->data_[idx - offset_] += *reinterpret_cast<const T*>(&(*it));
            std::advance(it, sizeof(T)/sizeof(Byte));
        }
    }
private:
    size_t offset_;
};
 
} /* woops */ 



#endif /* end of include guard: SERVER_STORAGE_H_Q4KYZGRA */
