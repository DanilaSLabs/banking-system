# Database format (data/database.json)

An example of a single entry:

```json
{
  "12345678": {
    "name": "Alice",
    "age": 30,
    "email": "alice@example.com",
    "secretWord": "rose",
    "accounts": [
      {
        "accId": 485701,
        "type": "Checking",
        "balance": 120.0
      },
      {
        "accId": 108824,
        "type": "Savings",
        "balance": 800.0,
        "savingsRate": 0.15,
        "lastSavedDate": "2025-11-04"
      }
    ]
  }
}

Client Fields
field     | Type    | Description
name      | string  | Customer name
age       | int     | Age
email     | string  | Mail
secretWord| string  | Password
accounts  | array   | Array of customer accounts


Account Fields
Field         | Type   | Description
accId         | int    | Unique Account ID
type          | string | Checking / Savings
balance       | double | Balance
savingsRate   | double | Interest rate
lastSavedDate | string | The date of the last interest accrual
