# rank_speed_test


### Compiler strangeness

Other compiler flags : `-DNDEBUG -pthread -mpopcnt -msse4.2 -O3 -DHAVE_ANSI_TERM -DHAVE_SSTREAM -Wall -pedantic -std=c++17`

GCC without `-march=native`

| sdsl (new style loop) | sdsl (old style loop) |
|-----------------------|----------|
| 3.09835s              | 4.52283s |
| 3.11792s              | 4.49202s |
| 3.09003s              | 4.50999s |

| compact (new style loop)         |  compact (old style loop) |
|-------|-----|
| 4.85654s | 11.9271s |
| 4.85307s | 11.9379s |
| 4.85646s | 11.916s |


GCC with `-march=native`

| sdsl (new style loop) | sdsl (old style loop) |
|-----------------------|----------|
| 3.25714s | 3.76658s |
| 3.25075s | 3.74863s |
| 3.24986s | 3.76629s |

| compact (new style loop) | compact (old style loop) |
|-----------------------|----------|
| 3.11286s | 10.2997s |
| 3.07636s | 10.3014s |
| 3.10178s | 10.3075s |


CLANG with `-march=native`

| sdsl (new style loop) | sdsl (old style loop) |
|-----------------------|----------|
| 4.27748s | 2.24296s |
| 4.27084s | 2.24488s | 
| 4.28095s | 2.27105s |

| compact (new style loop) | compact (old style loop) |
|-----------------------|----------|
| 9.26036s | 3.68063s |
| 9.24951s | 3.68437s |
| 9.2348s | 3.66826s |

CLANG without `-march=native`

| sdsl (new style loop) | sdsl (old style loop) |
|-----------------------|----------|
| 4.26805s | 2.54743s |
| 4.29264s | 2.52353s |
| 4.26444s | 2.55285s |

| compact (new style loop) | compact (old style loop) |
|-----------------------|----------|
| 10.2949s | 5.63155s |
| 10.2878s | 5.63871s |
| 10.2928s | 5.6348s |
