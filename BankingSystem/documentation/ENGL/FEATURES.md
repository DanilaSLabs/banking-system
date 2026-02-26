# Banking System Functionality

## 🧾 Working with clients
- Create, edit, delete clients
- ID and email verification
- Storage in JSON
- Automatic generation of unique account IDs

## 💰 Working with accounts
- Checking (current account)
- Savings (savings account, 15% rate)
- Accrual of interest on Savings
- Separate balance for each account

## , Operations
- **Deposit:** input of banknote denominations and quantity
- **Withdrawal:** checking the available balance
- **Translation:** Checking → other invoice by ID
- **Temporary contribution:** blocking the amount for N days

## 🔐 Security
- Storing data in JSON with backup
- Checking the correctness of the input
- Password recovery by email
- Protection against cleaning a non-empty database
- JSON and file system error handling

## 🧮 Interest support
- Simple Daily interest on Savings
- Automatic accrual upon entry
