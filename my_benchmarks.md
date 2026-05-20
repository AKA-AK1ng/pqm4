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
| viper-128 (10 executions) | clean | AVG: 637,468 <br /> MIN: 637,181 <br /> MAX: 637,687 | AVG: 861,615 <br /> MIN: 861,354 <br /> MAX: 861,811 | AVG: 1,047,122 <br /> MIN: 1,046,932 <br /> MAX: 1,047,323 |
| viper-192 (10 executions) | clean | AVG: 1,185,669 <br /> MIN: 1,185,397 <br /> MAX: 1,186,138 | AVG: 1,429,904 <br /> MIN: 1,429,600 <br /> MAX: 1,430,334 | AVG: 1,678,774 <br /> MIN: 1,678,468 <br /> MAX: 1,679,232 |
| viper-256 (10 executions) | clean | AVG: 1,860,947 <br /> MIN: 1,859,385 <br /> MAX: 1,870,624 | AVG: 2,149,038 <br /> MIN: 2,147,461 <br /> MAX: 2,158,693 | AVG: 2,472,688 <br /> MIN: 2,471,094 <br /> MAX: 2,482,347 |
| viper-384 (10 executions) | clean | AVG: 4,610,590 <br /> MIN: 4,601,282 <br /> MAX: 4,623,428 | AVG: 5,138,591 <br /> MIN: 5,129,305 <br /> MAX: 5,151,386 | AVG: 5,656,725 <br /> MIN: 5,647,376 <br /> MAX: 5,669,415 |
| viper-512 (10 executions) | clean | AVG: 7,192,548 <br /> MIN: 7,181,092 <br /> MAX: 7,224,715 | AVG: 7,823,369 <br /> MIN: 7,811,925 <br /> MAX: 7,855,613 | AVG: 8,503,943 <br /> MIN: 8,492,429 <br /> MAX: 8,536,043 |
| frost (10 executions) | frost128shake | AVG: 66,281,187 <br /> MIN: 66,281,187 <br /> MAX: 66,281,187 | AVG: 66,695,029 <br /> MIN: 66,695,029 <br /> MAX: 66,695,029 | AVG: 68,062,830 <br /> MIN: 68,062,830 <br /> MAX: 68,062,830 |
| firesaber (10 executions) | clean | AVG: 3,576,179 <br /> MIN: 3,576,179 <br /> MAX: 3,576,179 | AVG: 4,442,397 <br /> MIN: 4,442,397 <br /> MAX: 4,442,397 | AVG: 5,078,133 <br /> MIN: 5,078,133 <br /> MAX: 5,078,133 |
| firesaber (10 executions) | m4fspeed | AVG: 865,077 <br /> MIN: 865,077 <br /> MAX: 865,077 | AVG: 1,046,091 <br /> MIN: 1,046,091 <br /> MAX: 1,046,091 | AVG: 1,017,986 <br /> MIN: 1,017,986 <br /> MAX: 1,017,986 |
| firesaber (10 executions) | m4fstack | AVG: 1,190,264 <br /> MIN: 1,190,264 <br /> MAX: 1,190,264 | AVG: 1,456,600 <br /> MIN: 1,456,600 <br /> MAX: 1,456,600 | AVG: 1,462,402 <br /> MIN: 1,462,402 <br /> MAX: 1,462,402 |
| lightsaber (10 executions) | clean | AVG: 973,309 <br /> MIN: 973,309 <br /> MAX: 973,309 | AVG: 1,424,024 <br /> MIN: 1,424,024 <br /> MAX: 1,424,024 | AVG: 1,748,255 <br /> MIN: 1,748,255 <br /> MAX: 1,748,255 |
| lightsaber (10 executions) | m4fspeed | AVG: 305,992 <br /> MIN: 305,992 <br /> MAX: 305,992 | AVG: 416,563 <br /> MIN: 416,563 <br /> MAX: 416,563 | AVG: 401,807 <br /> MIN: 401,807 <br /> MAX: 401,807 |
| lightsaber (10 executions) | m4fstack | AVG: 376,226 <br /> MIN: 376,226 <br /> MAX: 376,226 | AVG: 526,885 <br /> MIN: 526,885 <br /> MAX: 526,885 | AVG: 530,268 <br /> MIN: 530,268 <br /> MAX: 530,268 |
| saber (10 executions) | clean | AVG: 2,040,005 <br /> MIN: 2,040,005 <br /> MAX: 2,040,005 | AVG: 2,726,476 <br /> MIN: 2,726,476 <br /> MAX: 2,726,476 | AVG: 3,221,340 <br /> MIN: 3,221,340 <br /> MAX: 3,221,340 |
| saber (10 executions) | m4fspeed | AVG: 559,745 <br /> MIN: 559,745 <br /> MAX: 559,745 | AVG: 710,636 <br /> MIN: 710,636 <br /> MAX: 710,636 | AVG: 684,930 <br /> MIN: 684,930 <br /> MAX: 684,930 |
| saber (10 executions) | m4fstack | AVG: 734,574 <br /> MIN: 734,574 <br /> MAX: 734,574 | AVG: 947,527 <br /> MIN: 947,527 <br /> MAX: 947,527 | AVG: 947,575 <br /> MIN: 947,575 <br /> MAX: 947,575 |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
| dilithium2 (10 executions) | m4f | AVG: 1,361,963 <br /> MIN: 1,345,411 <br /> MAX: 1,386,726 | AVG: 3,747,917 <br /> MIN: 1,719,980 <br /> MAX: 8,319,176 | AVG: 1,343,594 <br /> MIN: 1,343,296 <br /> MAX: 1,343,807 |
| dilithium3 (10 executions) | m4f | AVG: 2,394,739 <br /> MIN: 2,393,778 <br /> MAX: 2,395,569 | AVG: 5,689,854 <br /> MIN: 3,478,687 <br /> MAX: 8,251,330 | AVG: 2,289,937 <br /> MIN: 2,289,687 <br /> MAX: 2,290,155 |
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
| viper-128 | clean | 11,536 | 17,096 | 21,296 |
| viper-192 | clean | 17,504 | 26,624 | 31,856 |
| viper-256 | clean | 24,504 | 38,240 | 44,472 |
| viper-384 | clean | 51,776 | 85,576 | 94,880 |
| viper-512 | clean | 75,008 | 85,816 | 97,168 |
| frost | frost128shake | 27,020 | 69,220 | 79,660 |
| firesaber | clean | 19,368 | 22,512 | 23,992 |
| firesaber | m4fspeed | 7,664 | 8,336 | 8,344 |
| firesaber | m4fstack | 4,296 | 3,312 | 3,320 |
| lightsaber | clean | 9,332 | 11,452 | 12,196 |
| lightsaber | m4fspeed | 5,608 | 6,272 | 6,280 |
| lightsaber | m4fstack | 3,272 | 3,048 | 3,056 |
| saber | clean | 12,940 | 15,572 | 16,668 |
| saber | m4fspeed | 6,640 | 7,304 | 7,312 |
| saber | m4fstack | 3,784 | 3,176 | 3,184 |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
| dilithium2 | m4f | 38,264 | 49,344 | 36,176 |
| dilithium3 | m4f | 60,792 | 68,792 | 57,788 |
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
| viper-128 | clean | 46.5% | 40.6% | 33.4% |
| viper-192 | clean | 46.3% | 40.1% | 34.2% |
| viper-256 | clean | 46.1% | 40.6% | 35.3% |
| viper-384 | clean | 45.6% | 42.0% | 38.1% |
| viper-512 | clean | 45.7% | 42.7% | 39.3% |
| frost | frost128shake | 83.0% | 84.1% | 83.6% |
| firesaber | clean | 16.2% | 16.1% | 11.9% |
| firesaber | m4fspeed | 67.2% | 68.4% | 59.2% |
| firesaber | m4fstack | 48.9% | 49.2% | 41.2% |
| lightsaber | clean | 21.4% | 20.5% | 13.1% |
| lightsaber | m4fspeed | 68.1% | 69.9% | 57.1% |
| lightsaber | m4fstack | 55.5% | 55.3% | 43.3% |
| saber | clean | 18.8% | 18.2% | 12.5% |
| saber | m4fspeed | 68.5% | 69.9% | 59.0% |
| saber | m4fstack | 52.2% | 52.5% | 42.7% |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
| dilithium2 | m4f | 79.8% | 58.4% | 77.1% |
| dilithium3 | m4f | 82.3% | 63.4% | 79.6% |
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
| viper-128 | clean | 6,556 | 0 | 0 | 6,556 |
| viper-192 | clean | 6,988 | 0 | 0 | 6,988 |
| viper-256 | clean | 7,036 | 0 | 0 | 7,036 |
| viper-384 | clean | 6,692 | 0 | 0 | 6,692 |
| viper-512 | clean | 6,892 | 0 | 0 | 6,892 |
| frost | frost128shake | 10,872 | 0 | 0 | 10,872 |
| firesaber | clean | 10,000 | 0 | 0 | 10,000 |
| firesaber | m4fspeed | 18,768 | 0 | 0 | 18,768 |
| firesaber | m4fstack | 19,528 | 0 | 0 | 19,528 |
| lightsaber | clean | 10,184 | 0 | 0 | 10,184 |
| lightsaber | m4fspeed | 18,892 | 0 | 0 | 18,892 |
| lightsaber | m4fstack | 19,724 | 0 | 0 | 19,724 |
| saber | clean | 9,944 | 0 | 0 | 9,944 |
| saber | m4fspeed | 18,688 | 0 | 0 | 18,688 |
| saber | m4fstack | 19,428 | 0 | 0 | 19,428 |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| dilithium2 | m4f | 18,428 | 0 | 0 | 18,428 |
| dilithium3 | m4f | 19,904 | 0 | 0 | 19,904 |
