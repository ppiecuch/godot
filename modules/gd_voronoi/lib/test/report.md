## Timings Voronoi

| counts | jc_voronoi  | fastjet     | boost       |
|-------:|-------------|-------------|-------------|
| 10000  | 9.4470 ms   | 9.1300 ms   | 17.1540 ms  |
| 20000  | 20.4290 ms  | 18.9260 ms  | 35.2160 ms  |
| 30000  | 31.2910 ms  | 29.1090 ms  | 53.8950 ms  |
| 40000  | 41.7440 ms  | 39.6330 ms  | 73.4940 ms  |
| 50000  | 52.0670 ms  | 50.4690 ms  | 93.8770 ms  |
| 60000  | 63.3800 ms  | 62.4640 ms  | 114.0000 ms |
| 70000  | 73.4270 ms  | 74.0010 ms  | 135.0000 ms |
| 80000  | 83.2660 ms  | 85.6470 ms  | 155.0000 ms |
| 90000  | 93.9500 ms  | 99.1350 ms  | 175.0000 ms |
| 100000 | 105.0000 ms | 113.0000 ms | 202.0000 ms |
| 110000 | 115.0000 ms | 126.0000 ms | 226.0000 ms |
| 120000 | 126.0000 ms | 143.0000 ms | 247.0000 ms |
| 130000 | 140.0000 ms | 158.0000 ms | 267.0000 ms |
| 140000 | 150.0000 ms | 171.0000 ms | 291.0000 ms |
| 150000 | 162.0000 ms | 186.0000 ms | 315.0000 ms |
| 160000 | 174.0000 ms | 201.0000 ms | 335.0000 ms |
| 170000 | 184.0000 ms | 213.0000 ms | 357.0000 ms |
| 180000 | 196.0000 ms | 231.0000 ms | 380.0000 ms |
| 190000 | 208.0000 ms | 248.0000 ms | 402.0000 ms |
| 200000 | 217.0000 ms | 263.0000 ms | 425.0000 ms |
| 210000 | 228.0000 ms | 278.0000 ms | 443.0000 ms |
| 220000 | 243.0000 ms | 294.0000 ms | 468.0000 ms |
| 230000 | 254.0000 ms | 311.0000 ms | 492.0000 ms |
| 240000 | 272.0000 ms | 327.0000 ms | 514.0000 ms |
| 250000 | 278.0000 ms | 342.0000 ms | 539.0000 ms |
| 260000 | 287.0000 ms | 361.0000 ms | 561.0000 ms |
| 270000 | 305.0000 ms | 378.0000 ms | 586.0000 ms |
| 280000 | 312.0000 ms | 392.0000 ms | 609.0000 ms |
| 290000 | 326.0000 ms | 411.0000 ms | 633.0000 ms |
| 300000 | 341.0000 ms | 429.0000 ms | 653.0000 ms |


## Memory Voronoi

| counts | jc_voronoi | fastjet  | boost     |
|-------:|------------|----------|-----------|
| 10000  | 4440 kb    | 1372 kb  | 9383 kb   |
| 20000  | 8865 kb    | 2739 kb  | 18775 kb  |
| 30000  | 13289 kb   | 4105 kb  | 26865 kb  |
| 40000  | 17698 kb   | 5470 kb  | 37509 kb  |
| 50000  | 22122 kb   | 6835 kb  | 45588 kb  |
| 60000  | 26531 kb   | 8199 kb  | 53704 kb  |
| 70000  | 30939 kb   | 9562 kb  | 66900 kb  |
| 80000  | 35348 kb   | 10923 kb | 74979 kb  |
| 90000  | 39756 kb   | 12283 kb | 83046 kb  |
| 100000 | 44149 kb   | 13642 kb | 91105 kb  |
| 110000 | 48541 kb   | 15000 kb | 99162 kb  |
| 120000 | 52950 kb   | 16361 kb | 107223 kb |
| 130000 | 57326 kb   | 17717 kb | 115271 kb |
| 140000 | 61719 kb   | 19074 kb | 133560 kb |
| 150000 | 66111 kb   | 20429 kb | 141619 kb |
| 160000 | 70488 kb   | 21785 kb | 149672 kb |
| 170000 | 74880 kb   | 23139 kb | 157712 kb |
| 180000 | 79241 kb   | 24491 kb | 165751 kb |
| 190000 | 83617 kb   | 25842 kb | 173784 kb |
| 200000 | 87978 kb   | 27191 kb | 181811 kb |
| 210000 | 92338 kb   | 28541 kb | 189829 kb |
| 220000 | 96715 kb   | 29894 kb | 197884 kb |
| 230000 | 101059 kb  | 31240 kb | 205896 kb |
| 240000 | 105436 kb  | 32589 kb | 213920 kb |
| 250000 | 109780 kb  | 33936 kb | 221935 kb |
| 260000 | 114125 kb  | 35281 kb | 229938 kb |
| 270000 | 118485 kb  | 36627 kb | 258422 kb |
| 280000 | 122846 kb  | 37974 kb | 266441 kb |
| 290000 | 127174 kb  | 39320 kb | 274450 kb |
| 300000 | 131519 kb  | 40664 kb | 282439 kb |


## Num Allocations Voronoi

| counts | jc_voronoi | fastjet | boost   |
|-------:|------------|---------|---------|
| 10000  | 258        | 30385   | 55797   |
| 20000  | 515        | 60506   | 111734  |
| 30000  | 772        | 90573   | 167559  |
| 40000  | 1028       | 120592  | 223313  |
| 50000  | 1285       | 150610  | 278976  |
| 60000  | 1541       | 180599  | 334736  |
| 70000  | 1797       | 210570  | 390348  |
| 80000  | 2053       | 240485  | 446084  |
| 90000  | 2309       | 270374  | 501602  |
| 100000 | 2564       | 300247  | 556989  |
| 110000 | 2819       | 330100  | 612348  |
| 120000 | 3075       | 360013  | 667721  |
| 130000 | 3329       | 389811  | 722965  |
| 140000 | 3584       | 419623  | 778208  |
| 150000 | 3839       | 449397  | 833652  |
| 160000 | 4093       | 479188  | 888981  |
| 170000 | 4348       | 508944  | 944106  |
| 180000 | 4601       | 538650  | 999242  |
| 190000 | 4855       | 568350  | 1054283 |
| 200000 | 5108       | 597989  | 1109259 |
| 210000 | 5361       | 627652  | 1164060 |
| 220000 | 5615       | 657370  | 1219475 |
| 230000 | 5867       | 686944  | 1274236 |
| 240000 | 6121       | 716594  | 1329141 |
| 250000 | 6373       | 746185  | 1383938 |
| 260000 | 6625       | 775751  | 1438538 |
| 270000 | 6878       | 805313  | 1493161 |
| 280000 | 7131       | 834912  | 1548022 |
| 290000 | 7382       | 864488  | 1602718 |
| 300000 | 7634       | 894015  | 1657097 |


# Report made in 0.001533 seconds