# Speed Evaluation
## Key Encapsulation Schemes
| scheme | implementation | key generation [cycles] | encapsulation [cycles] | decapsulation [cycles] |
| ------ | -------------- | ----------------------- | ---------------------- | ---------------------- |
| frodo1344 (10 executions) | frodo1344kemaes | AVG: 217,208,192 <br /> MIN: 217,208,162 <br /> MAX: 217,208,209 | AVG: 210,884,359 <br /> MIN: 210,884,347 <br /> MAX: 210,884,390 | AVG: 209,718,012 <br /> MIN: 209,718,008 <br /> MAX: 209,718,018 |
| frodo1344 (10 executions) | frodo1344kemshake | AVG: 292,225,192 <br /> MIN: 292,225,142 <br /> MAX: 292,225,224 | AVG: 291,693,698 <br /> MIN: 291,693,681 <br /> MAX: 291,693,731 | AVG: 290,527,323 <br /> MIN: 290,527,302 <br /> MAX: 290,527,347 |
| frodo640 (10 executions) | frodokem640aes | AVG: 51,667,013 <br /> MIN: 51,667,011 <br /> MAX: 51,667,017 | AVG: 50,483,284 <br /> MIN: 50,483,280 <br /> MAX: 50,483,289 | AVG: 50,015,247 <br /> MIN: 50,015,245 <br /> MAX: 50,015,249 |
| frodo640 (10 executions) | frodokem640shake | AVG: 70,927,342 <br /> MIN: 70,927,285 <br /> MAX: 70,927,437 | AVG: 70,538,467 <br /> MIN: 70,538,455 <br /> MAX: 70,538,501 | AVG: 70,070,433 <br /> MIN: 70,070,424 <br /> MAX: 70,070,464 |
| frodo976 (10 executions) | frodokem976aes | AVG: 116,156,058 <br /> MIN: 116,156,023 <br /> MAX: 116,156,072 | AVG: 113,284,643 <br /> MIN: 113,284,609 <br /> MAX: 113,284,659 | AVG: 112,450,527 <br /> MIN: 112,450,495 <br /> MAX: 112,450,539 |
| frodo976 (10 executions) | frodokem976shake | AVG: 159,596,771 <br /> MIN: 159,596,708 <br /> MAX: 159,596,849 | AVG: 159,776,428 <br /> MIN: 159,776,335 <br /> MAX: 159,776,556 | AVG: 158,942,289 <br /> MIN: 158,942,209 <br /> MAX: 158,942,368 |
| frost-128 (10 executions) | frost128aes | AVG: 41,864,953 <br /> MIN: 41,864,933 <br /> MAX: 41,864,975 | AVG: 41,207,731 <br /> MIN: 41,207,723 <br /> MAX: 41,207,767 | AVG: 42,420,862 <br /> MIN: 42,420,825 <br /> MAX: 42,420,870 |
| frost-128 (10 executions) | frost128shake | AVG: 56,090,120 <br /> MIN: 56,090,067 <br /> MAX: 56,090,156 | AVG: 57,157,413 <br /> MIN: 57,157,377 <br /> MAX: 57,157,455 | AVG: 58,370,504 <br /> MIN: 58,370,457 <br /> MAX: 58,370,558 |
| frost-192 (10 executions) | frost192aes | AVG: 115,618,485 <br /> MIN: 115,618,451 <br /> MAX: 115,618,499 | AVG: 112,209,222 <br /> MIN: 112,209,191 <br /> MAX: 112,209,235 | AVG: 114,018,361 <br /> MIN: 114,018,328 <br /> MAX: 114,018,372 |
| frost-192 (10 executions) | frost192shake | AVG: 158,436,781 <br /> MIN: 158,436,759 <br /> MAX: 158,436,808 | AVG: 158,996,660 <br /> MIN: 158,996,595 <br /> MAX: 158,996。,703 | AVG: 160,805,805 <br /> MIN: 160,805,755 <br /> MAX: 160,805,853 |
| frost-256 (10 executions) | frost256aes | AVG: 212,991,798 <br /> MIN: 212,991,770 <br /> MAX: 212,991,820 | AVG: 205,537,264 <br /> MIN: 205,537,251 <br /> MAX: 205,537,292 | AVG: 207,717,383 <br /> MIN: 207,717,360 <br /> MAX: 207,717,411 |
| frost-256 (10 executions) | frost256shake | AVG: 285,770,338 <br /> MIN: 285,770,330 <br /> MAX: 285,770,345 | AVG: 287,389,876 <br /> MIN: 287,389,823 <br /> MAX: 287,389,974 | AVG: 289,569,955 <br /> MIN: 289,569,834 <br /> MAX: 289,570,029 |
## Signature Schemes
| scheme | implementation | key generation [cycles] | sign [cycles] | verify [cycles] |
| ------ | -------------- | ----------------------- | ------------- | --------------- |
# Memory Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [bytes] | Encapsulation [bytes] | Decapsulation [bytes] |
| ------ | -------------- | ---------------------- | --------------------- | --------------------- |
| frodo1344 | frodo1344kemaes | 65,828 | 130,812 | 173,948 |
| frodo1344 | frodo1344kemshake | 54,620 | 108,852 | 151,988 |
| frodo640 | frodokem640aes | 31,980 | 62,476 | 83,100 |
| frodo640 | frodokem640shake | 26,404 | 51,780 | 72,404 |
| frodo976 | frodokem976aes | 48,140 | 95,420 | 126,796 |
| frodo976 | frodokem976shake | 39,876 | 79,348 | 110,724 |
| frost-128 | frost128aes | 28,956 | 72,108 | 81,516 |
| frost-128 | frost128shake | 23,908 | 62,452 | 71,860 |
| frost-192 | frost192aes | 47,812 | 120,956 | 136,644 |
| frost-192 | frost192shake | 39,636 | 105,028 | 120,716 |
| frost-256 | frost256aes | 64,748 | 166,964 | 188,300 |
| frost-256 | frost256shake | 53,756 | 145,404 | 166,740 |
## Signature Schemes
| Scheme | Implementation | Key Generation [bytes] | Sign [bytes] | Verify [bytes] |
| ------ | -------------- | ---------------------- | ------------ | -------------- |
# Hashing Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | Key Generation [%] | Encapsulation [%] | Decapsulation [%] |
| ------ | -------------- | ------------------ | ----------------- | ----------------- |
| frodo1344 | frodo1344kemaes | 75.3% | 78.3% | 78.0% |
| frodo1344 | frodo1344kemshake | 81.8% | 82.4% | 82.2% |
| frodo640 | frodokem640aes | 73.8% | 76.8% | 76.2% |
| frodo640 | frodokem640shake | 81.5% | 82.9% | 82.5% |
| frodo976 | frodokem976aes | 75.0% | 77.8% | 77.5% |
| frodo976 | frodokem976shake | 81.9% | 82.5% | 82.2% |
| frost-128 | frost128aes | 73.7% | 77.3% | 77.1% |
| frost-128 | frost128shake | 81.6% | 81.9% | 81.6% |
| frost-192 | frost192aes | 74.0% | 78.0% | 77.8% |
| frost-192 | frost192shake | 81.7% | 82.6% | 82.4% |
| frost-256 | frost256aes | 74.1% | 78.2% | 78.0% |
| frost-256 | frost256shake | 82.0% | 82.5% | 82.4% |
## Signature Schemes
| Scheme | Implementation | Key Generation [%] | Sign [%] | Verify [%] |
| ------ | -------------- | ------------------ | -------- | ---------- |
# Size Evaluation
## Key Encapsulation Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
| frodo1344 | frodo1344kemaes | 10,636 | 0 | 0 | 10,636 |
| frodo1344 | frodo1344kemshake | 10,688 | 0 | 0 | 10,688 |
| frodo640 | frodokem640aes | 8,904 | 0 | 0 | 8,904 |
| frodo640 | frodokem640shake | 8,980 | 0 | 0 | 8,980 |
| frodo976 | frodokem976aes | 9,572 | 0 | 0 | 9,572 |
| frodo976 | frodokem976shake | 9,612 | 0 | 0 | 9,612 |
| frost-128 | frost128aes | 9,744 | 0 | 0 | 9,744 |
| frost-128 | frost128shake | 9,868 | 0 | 0 | 9,868 |
| frost-192 | frost192aes | 11,096 | 0 | 0 | 11,096 |
| frost-192 | frost192shake | 11,188 | 0 | 0 | 11,188 |
| frost-256 | frost256aes | 12,240 | 0 | 0 | 12,240 |
| frost-256 | frost256shake | 12,320 | 0 | 0 | 12,320 |
## Signature Schemes
| Scheme | Implementation | .text [bytes] | .data [bytes] | .bss [bytes] | Total [bytes] |
| ------ | -------------- | ------------- | ------------- | ------------ | ------------- |
