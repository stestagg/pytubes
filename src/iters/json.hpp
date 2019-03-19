#pragma once

#include "../util/json/json.hpp"

namespace ss{ namespace iter{

    class JsonParseIter : public Iter {

        /*<-
        Iter:
            JsonParseIter: [AnyIter]
        Tube:
            JsonParse:
                props: [
                    {type: Tube, name: parent, dtypes: [Utf8]},
                ]
                dtype: return (JsonUtf8,)
                iter: [JsonParseIter, [parent.iter]]
    ->*/
        const Utf8 *source;
        JsonUtf8 current_value = json::Value<uint8_t>();
        SlotPointer slot;

    public:
        JsonParseIter(AnyIter parent)
            : source(parent->get_slots()[0]),
              slot(&current_value)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next() {
            current_value = json::tokenize_entire<uint8_t>(*source);
        }

    };

}}