*Benchmark of 1024*1024 vector of int32_t `min` function BLAS*

| function props |  Naive  |  Neon lib  |  FPGA  | 
| --- | --- | --- | --- |
| 100Mhz | 50K[us] | 4K[us] | 3.5K[us]
| 300Mhz | 35K[us] | 4K[us] | 3.5K[us]
| 300Mhz,bus128 | [us] |


Naive and Neon don't change fw clock, compiled with `-o2` optimization


Intel i7-6700 | 3.5GHz
vector(heap) 15K[us] 
array(stack) 5K[us] 
