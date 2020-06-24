*Benchmark of 1024*1024 vector of int32_t `min` function BLAS*

| function props |  Naive  |  Neon lib  |  FPGA  | 
| --- | --- | --- | --- |
| 100Mhz | 50K[us] |
| 300Mhz | 35K[us] |
| 300Mhz,bus128 | [us] |





Intel i7-6700 | 3.5GHz
vector(heap) 15K[us] 
array(stack) 5K[us] 
