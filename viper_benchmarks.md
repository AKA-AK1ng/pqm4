# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
| viper-af67-128 (10 executions) | m4fspeed | AVG: 373,249 <br /> MIN: 373,177 <br /> MAX: 373,281 | AVG: 539,612 <br /> MIN: 539,580 <br /> MAX: 539,646 | AVG: 2,127,314 <br /> MIN: 2,127,274 <br /> MAX: 2,127,371 |
| viper-af67-128 (10 executions) | m4fstack | AVG: 392,586 <br /> MIN: 392,504 <br /> MAX: 392,672 | AVG: 579,810 <br /> MIN: 579,728 <br /> MAX: 579,893 | AVG: 2,167,508 <br /> MIN: 2,167,418 <br /> MAX: 2,167,601 |
| viper-af67-128 (10 executions) | ref | AVG: 1,061,193 <br /> MIN: 1,061,193 <br /> MAX: 1,061,194 | AVG: 1,589,777 <br /> MIN: 1,589,773 <br /> MAX: 1,589,813 | AVG: 1,991,136 <br /> MIN: 1,991,133 <br /> MAX: 1,991,170 |
| viper-af67-192 (10 executions) | m4fspeed | AVG: 719,616 <br /> MIN: 719,587 <br /> MAX: 719,648 | AVG: 932,302 <br /> MIN: 932,266 <br /> MAX: 932,355 | AVG: 1,019,234 <br /> MIN: 1,019,209 <br /> MAX: 1,019,261 |
| viper-af67-192 (10 executions) | m4fstack | AVG: 799,416 <br /> MIN: 799,380 <br /> MAX: 799,444 | AVG: 1,050,125 <br /> MIN: 1,050,082 <br /> MAX: 1,050,184 | AVG: 1,136,598 <br /> MIN: 1,136,547 <br /> MAX: 1,136,632 |
| viper-af67-192 (10 executions) | ref | AVG: 2,251,751 <br /> MIN: 2,251,747 <br /> MAX: 2,251,787 | AVG: 2,994,770 <br /> MIN: 2,994,763 <br /> MAX: 2,994,801 | AVG: 3,583,040 <br /> MIN: 3,583,033 <br /> MAX: 3,583,073 |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
| viper-af67-128 | m4fspeed | 12,200 | 14,552 | 14,528 |
| viper-af67-128 | m4fstack | 9,976 | 12,336 | 12,312 |
| viper-af67-128 | ref | 10,932 | 14,812 | 15,172 |
| viper-af67-192 | m4fspeed | 17,360 | 20,192 | 20,168 |
| viper-af67-192 | m4fstack | 14,104 | 16,944 | 16,920 |
| viper-af67-192 | ref | 17,140 | 22,060 | 22,452 |
| viper-af67-256 | m4fspeed | 23,504 | 26,848 | 26,824 |
| viper-af67-256 | m4fstack | 19,224 | 22,576 | 22,552 |
| viper-af67-256 | ref | 24,500 | 30,956 | 31,412 |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
| viper-af67-128 | m4fspeed | 64.5% | 69.1% | 16.0% |
| viper-af67-128 | m4fstack | 61.3% | 64.3% | 15.7% |
| viper-af67-128 | ref | 22.7% | 23.5% | 17.1% |
| viper-af67-192 | m4fspeed | 59.4% | 62.4% | 51.7% |
| viper-af67-192 | m4fstack | 53.5% | 55.4% | 46.4% |
| viper-af67-192 | ref | 19.0% | 19.5% | 14.7% |
| viper-af67-256 | m4fspeed | 59.8% | 61.8% | 52.4% |
| viper-af67-256 | m4fstack | 52.7% | 53.8% | 46.1% |
| viper-af67-256 | ref | 17.9% | 18.0% | 14.2% |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| viper-af67-128 | m4fspeed | 27,656 | 0 | 0 | 27,656 |
| viper-af67-128 | m4fstack | 26,720 | 0 | 0 | 26,720 |
| viper-af67-128 | ref | 15,412 | 0 | 0 | 15,412 |
| viper-af67-192 | m4fspeed | 28,544 | 0 | 0 | 28,544 |
| viper-af67-192 | m4fstack | 27,400 | 0 | 0 | 27,400 |
| viper-af67-192 | ref | 15,592 | 0 | 0 | 15,592 |
| viper-af67-256 | m4fspeed | 28,696 | 0 | 0 | 28,696 |
| viper-af67-256 | m4fstack | 27,872 | 0 | 0 | 27,872 |
| viper-af67-256 | ref | 15,996 | 0 | 0 | 15,996 |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
