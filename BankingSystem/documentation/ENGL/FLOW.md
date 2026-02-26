# The logic of the Banking System

##1. Main Menu
After startup, the `DatabaseManager("data/database") object is created.json")`.
Then the menu is displayed:

| Log in (l)
| Create an account (c)
| Forgot password (f)
| Exit (e)


---

## 2. Creating a client
1. Enter your name, age, email, ID, and secret word.  
2. ID verification (only numbers, length 8-10).
3. If the ID already exists— go to login.
4. Select the number of accounts:
- `1 → Checking`;
- `2 → Checking + Savings (15% rate)'.  
5. The data is saved in JSON.

---

## 3. Entrance
1. Enter the ID and the secret word.  
2. Check through the 'DatabaseManager::loadCustomer'.  
3. Upon successful entry, interest is accrued on Savings.
4. The client enters the personal menu.

---

## 4. The client's menu

    Deposit (ATM)

    Withdraw

    Create time deposit (Savings only)

    Transfer between accounts (only from Checking)

    Logout


- **Deposit** — input of denomination and number of bills, verification.
- **Withdraw** — withdraw funds with a balance check.
- **Transfer** — transfer by `accId` between any clients.
- **Time deposit** — deposit simulation (reduces Savings balance).
- **Logout** — save the client and exit to the main menu.

---

##5. Forgot your password
1. Enter your ID and email address.  
2. Checking for a match.
3. Installing a new `secretWord'.  
4. Saving in JSON.

---

##6. Interest accrual
When logging in, the program checks:
- if `type == "Savings"`;
- if days have passed since `lastSavedDate`;
- charges simple interest:  
  `interest = balance * rate * (days / 365.0)`;
- updates the `lastSavedDate'.
