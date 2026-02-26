# Testing the Banking System

## 🧪 Automatic tests
Implemented in `tests.cpp `. The `data/test_db' test database is used.json`.

### Coverage
| No. | Test | Checks |
|---|------|-----------|
| 1 | Creating and uploading a client | writing/reading JSON |
| 2 | Updating the client | that other records are not affected |
| 3 | Deletion | correct deletion from the database |
| 4 | Checking/Savings | rule 1 or 2 accounts |
| 5 | Deposit/Withdrawal | balance correctness |
| 6 | Translation | both sides are saved |
| 7 | Interest | accrual on Savings |
| 8 | Unique ID | generating a new number |
| 9 | Password reset | Email recovery |
| 10 | Backup | creation .bak |
| 11 | Broken JSON | correct error handling |

---

## 🧍‍♂️ Manual tests
1. Create a client with 1 account — check that the Checking account has been created.
2. Create a client with 2 accounts — check Checking and Savings.
3. Log in → make a deposit → check the balance.
4. Withdraw more than there is → get rejected.
5. Transfer from Checking to Savings.
6. Check the `database.json` and the presence of `database.json.bak'.
7. Check password recovery by email.
