## 2026-06-10
- first benchmark with basic dfs and 8 scramble moves
- added OpenMP parallelization to the first level of the search tree

20260602u

|Date|No OpenMP|Scramble Moves|Seed
|-|-|-|-|
|2026-06-10|0m51.856s|8|20260602u|
|2026-06-10|0m53.501s|8|20260602u|
|2026-06-10|0m1.487s|9|20260602u|
|2026-06-10|57m45.625s|9|123|

|Date|OpenMP|Scramble Moves|Threads|Seed|
|-|-|-|-|-|
|2026-06-10|0m22.954s|8|11|20260602u|
|2026-06-10|0m37.633s|8|18|20260602u|
|2026-06-10|1m56.042s|9|11|20260602u|



## Learnings
- openMP Tasks 
- dfs was not canceled so every thread keept working, thats why we had faster serial paths for some seed that had an early solution