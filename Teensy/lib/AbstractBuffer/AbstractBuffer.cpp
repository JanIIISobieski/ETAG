#include <AbstractBuffer.h>

template class AbstractBuffer<int16_t>;
template class AbstractBuffer<uint16_t>;
template class AbstractBuffer<uint8_t>;

template <class T>
AbstractBuffer<T>::AbstractBuffer(volatile T** buff_vec, size_t num_buffers, size_t length_buffers, uint8_t id) : BufferBase(num_buffers, length_buffers, id) {
    size_t length_diff_lookup_table[9] = {0, 6, 3, 0, 2, 0, 0, 0, 1}; //non-zero indices correspond to sizeof u/int8_t, u/int16_t, u/int32_t, u/int64_t
    
    buffers = new BufferHelper<T>[num_buffers];  // This requires calling delete in the destructor (though none of these classes are deallocated during runtime)

    for (size_t i = 0; i < num_buffers; i++) {
        //Allocate the pointers for each buffer in the buffer helper array
        buffers[i].head_ptr = buff_vec[i];
        buffers[i].length = length_buffers;
        buffers[i].id_ptr = reinterpret_cast<volatile uint8_t*>(buff_vec[i]);
        buffers[i].count_ptr = buffers[i].id_ptr + 1;
        buffers[i].time_ptr = reinterpret_cast<volatile uint32_t*>(buffers[i].id_ptr + 2);
        buffers[i].data_ptr = reinterpret_cast<volatile T*>(buffers[i].id_ptr + 6);
        buffers[i].data_length = length_buffers - length_diff_lookup_table[sizeof(T)];

        //Assign the ID, count, and time to each buffer
        *(buffers[i].id_ptr) = identifier;
        *(buffers[i].count_ptr) = 0;
        *(buffers[i].time_ptr) = 0;
    }
}

template <class T>
AbstractBuffer<T>::~AbstractBuffer() {
    delete[] buffers;
}
