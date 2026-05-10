# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
| m-lwq-1024 (10 executions) | clean | AVG: 1,813,610 <br /> MIN: 1,812,201 <br /> MAX: 1,822,987 | AVG: 2,153,457 <br /> MIN: 2,152,007 <br /> MAX: 2,162,813 | AVG: 2,371,340 <br /> MIN: 2,369,830 <br /> MAX: 2,380,708 |
| m-lwq-512 (10 executions) | clean | AVG: 636,173 <br /> MIN: 635,768 <br /> MAX: 636,597 | AVG: 857,447 <br /> MIN: 857,043 <br /> MAX: 857,825 | AVG: 979,838 <br /> MIN: 979,452 <br /> MAX: 980,209 |
| m-lwq-768 (10 executions) | clean | AVG: 1,148,715 <br /> MIN: 1,147,144 <br /> MAX: 1,158,620 | AVG: 1,431,081 <br /> MIN: 1,429,499 <br /> MAX: 1,440,947 | AVG: 1,601,247 <br /> MIN: 1,599,668 <br /> MAX: 1,611,098 |
| ml-kem-1024 (10 executions) | clean | AVG: 1,422,805 <br /> MIN: 1,421,323 <br /> MAX: 1,432,368 | AVG: 1,613,740 <br /> MIN: 1,612,258 <br /> MAX: 1,623,312 | AVG: 1,894,385 <br /> MIN: 1,892,903 <br /> MAX: 1,903,957 |
| ml-kem-1024 (10 executions) | m4fspeed | AVG: 971,528 <br /> MIN: 971,046 <br /> MAX: 971,864 | AVG: 983,086 <br /> MIN: 982,579 <br /> MAX: 983,418 | AVG: 1,044,876 <br /> MIN: 1,044,369 <br /> MAX: 1,045,208 |
| ml-kem-1024 (10 executions) | m4fstack | AVG: 976,571 <br /> MIN: 973,970 <br /> MAX: 985,548 | AVG: 992,525 <br /> MIN: 989,924 <br /> MAX: 1,001,502 | AVG: 1,055,101 <br /> MIN: 1,052,500 <br /> MAX: 1,064,078 |
| ml-kem-512 (10 executions) | clean | AVG: 547,623 <br /> MIN: 547,476 <br /> MAX: 547,778 | AVG: 658,499 <br /> MIN: 658,352 <br /> MAX: 658,654 | AVG: 828,560 <br /> MIN: 828,413 <br /> MAX: 828,715 |
| ml-kem-512 (10 executions) | m4fspeed | AVG: 376,074 <br /> MIN: 375,626 <br /> MAX: 376,433 | AVG: 375,153 <br /> MIN: 374,705 <br /> MAX: 375,512 | AVG: 411,265 <br /> MIN: 410,817 <br /> MAX: 411,624 |
| ml-kem-512 (10 executions) | m4fstack | AVG: 375,922 <br /> MIN: 375,446 <br /> MAX: 376,206 | AVG: 376,934 <br /> MIN: 376,459 <br /> MAX: 377,219 | AVG: 413,292 <br /> MIN: 412,817 <br /> MAX: 413,577 |
| ml-kem-768 (10 executions) | clean | AVG: 915,127 <br /> MIN: 914,889 <br /> MAX: 915,402 | AVG: 1,069,617 <br /> MIN: 1,069,379 <br /> MAX: 1,069,892 | AVG: 1,292,747 <br /> MIN: 1,292,509 <br /> MAX: 1,293,022 |
| ml-kem-768 (10 executions) | m4fspeed | AVG: 613,086 <br /> MIN: 611,550 <br /> MAX: 623,259 | AVG: 629,756 <br /> MIN: 628,211 <br /> MAX: 639,947 | AVG: 677,392 <br /> MIN: 675,847 <br /> MAX: 687,583 |
| ml-kem-768 (10 executions) | m4fstack | AVG: 614,988 <br /> MIN: 613,447 <br /> MAX: 624,749 | AVG: 634,914 <br /> MIN: 633,373 <br /> MAX: 644,675 | AVG: 683,219 <br /> MIN: 681,678 <br /> MAX: 692,980 |
| frodo640 (10 executions) | frodokem640shake | AVG: 67,074,255 <br /> MIN: 67,074,255 <br /> MAX: 67,074,255 | AVG: 66,522,019 <br /> MIN: 66,522,019 <br /> MAX: 66,522,019 | AVG: 66,076,104 <br /> MIN: 66,076,104 <br /> MAX: 66,076,104 |
| venom (10 executions) | venom640shake | AVG: 66,251,053 <br /> MIN: 66,251,053 <br /> MAX: 66,251,053 | AVG: 66,486,475 <br /> MIN: 66,486,475 <br /> MAX: 66,486,475 | AVG: 67,080,505 <br /> MIN: 67,080,505 <br /> MAX: 67,080,505 |
| viper-128 (10 executions) | clean | AVG: 637,394 <br /> MIN: 637,101 <br /> MAX: 637,583 | AVG: 892,912 <br /> MIN: 892,621 <br /> MAX: 893,107 | AVG: 1,079,907 <br /> MIN: 1,079,607 <br /> MAX: 1,080,076 |
| viper-192 (10 executions) | clean | AVG: 1,186,643 <br /> MIN: 1,185,206 <br /> MAX: 1,196,160 | AVG: 1,550,920 <br /> MIN: 1,549,435 <br /> MAX: 1,560,437 | AVG: 1,806,610 <br /> MIN: 1,805,104 <br /> MAX: 1,816,129 |
| viper-256 (10 executions) | clean | AVG: 1,862,209 <br /> MIN: 1,859,804 <br /> MAX: 1,871,256 | AVG: 2,395,469 <br /> MIN: 2,393,036 <br /> MAX: 2,404,507 | AVG: 2,724,300 <br /> MIN: 2,721,847 <br /> MAX: 2,733,303 |
| viper-384 (10 executions) | clean | AVG: 4,608,359 <br /> MIN: 4,601,190 <br /> MAX: 4,633,703 | AVG: 6,032,113 <br /> MIN: 6,024,976 <br /> MAX: 6,057,467 | AVG: 6,583,918 <br /> MIN: 6,576,633 <br /> MAX: 6,609,271 |
| viper-512 (10 executions) | clean | AVG: 7,186,954 <br /> MIN: 7,181,104 <br /> MAX: 7,192,652 | AVG: 9,402,982 <br /> MIN: 9,397,221 <br /> MAX: 9,408,616 | AVG: 10,099,328 <br /> MIN: 10,093,704 <br /> MAX: 10,104,840 |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
| m-lwq-1024 | clean | 24,496 | 38,744 | 43,664 |
| m-lwq-512 | clean | 11,536 | 17,608 | 21,136 |
| m-lwq-768 | clean | 17,504 | 27,136 | 31,376 |
| ml-kem-1024 | clean | 15,136 | 18,776 | 20,352 |
| ml-kem-1024 | m4fspeed | 6,436 | 7,500 | 7,484 |
| ml-kem-1024 | m4fstack | 3,332 | 3,372 | 3,364 |
| ml-kem-512 | clean | 6,160 | 8,792 | 9,568 |
| ml-kem-512 | m4fspeed | 4,372 | 5,436 | 5,412 |
| ml-kem-512 | m4fstack | 2,300 | 2,348 | 2,332 |
| ml-kem-768 | clean | 10,248 | 13,384 | 14,480 |
| ml-kem-768 | m4fspeed | 5,396 | 6,468 | 6,452 |
| ml-kem-768 | m4fstack | 2,828 | 2,868 | 2,852 |
| frodo640 | frodokem640shake | 26,404 | 51,780 | 72,404 |
| venom | venom640shake | 27,020 | 47,940 | 68,596 |
| viper-128 | clean | 11,536 | 15,056 | 19,256 |
| viper-192 | clean | 17,504 | 22,032 | 27,264 |
| viper-256 | clean | 24,504 | 30,056 | 36,288 |
| viper-384 | clean | 51,776 | 60,512 | 69,816 |
| viper-512 | clean | 75,008 | 85,972 | 97,224 |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
| m-lwq-1024 | clean | 46.6% | 41.7% | 34.4% |
| m-lwq-512 | clean | 48.0% | 41.7% | 31.2% |
| m-lwq-768 | clean | 47.0% | 41.4% | 33.1% |
| ml-kem-1024 | clean | 51.4% | 46.0% | 39.2% |
| ml-kem-1024 | m4fspeed | 74.9% | 75.2% | 70.8% |
| ml-kem-1024 | m4fstack | 74.7% | 74.6% | 70.3% |
| ml-kem-512 | clean | 51.6% | 41.6% | 33.1% |
| ml-kem-512 | m4fspeed | 75.0% | 72.9% | 66.6% |
| ml-kem-512 | m4fstack | 75.0% | 72.5% | 66.2% |
| ml-kem-768 | clean | 49.7% | 43.6% | 36.0% |
| ml-kem-768 | m4fspeed | 74.1% | 73.9% | 68.7% |
| ml-kem-768 | m4fstack | 73.9% | 73.3% | 68.1% |
| frodo640 | frodokem640shake | 82.2% | 83.8% | 83.4% |
| venom | venom640shake | 83.0% | 84.2% | 83.8% |
| viper-128 | clean | 46.4% | 38.0% | 31.4% |
| viper-192 | clean | 46.4% | 36.4% | 31.3% |
| viper-256 | clean | 46.1% | 36.0% | 31.7% |
| viper-384 | clean | 45.6% | 35.6% | 32.7% |
| viper-512 | clean | 45.7% | 35.4% | 33.0% |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| m-lwq-1024 | clean | 6,696 | 0 | 0 | 6,696 |
| m-lwq-512 | clean | 6,472 | 0 | 0 | 6,472 |
| m-lwq-768 | clean | 6,632 | 0 | 0 | 6,632 |
| ml-kem-1024 | clean | 6,220 | 0 | 0 | 6,220 |
| ml-kem-1024 | m4fspeed | 16,756 | 0 | 0 | 16,756 |
| ml-kem-1024 | m4fstack | 13,900 | 0 | 0 | 13,900 |
| ml-kem-512 | clean | 5,232 | 0 | 0 | 5,232 |
| ml-kem-512 | m4fspeed | 15,744 | 0 | 0 | 15,744 |
| ml-kem-512 | m4fstack | 13,232 | 0 | 0 | 13,232 |
| ml-kem-768 | clean | 5,240 | 0 | 0 | 5,240 |
| ml-kem-768 | m4fspeed | 15,944 | 0 | 0 | 15,944 |
| ml-kem-768 | m4fstack | 13,220 | 0 | 0 | 13,220 |
| frodo640 | frodokem640shake | 8,980 | 0 | 0 | 8,980 |
| venom | venom640shake | 10,556 | 0 | 0 | 10,556 |
| viper-128 | clean | 6,500 | 0 | 0 | 6,500 |
| viper-192 | clean | 6,864 | 0 | 0 | 6,864 |
| viper-256 | clean | 6,816 | 0 | 0 | 6,816 |
| viper-384 | clean | 6,656 | 0 | 0 | 6,656 |
| viper-512 | clean | 6,712 | 0 | 0 | 6,712 |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
