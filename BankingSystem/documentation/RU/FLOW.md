# Логика работы Banking System

## 1. Главное меню
После запуска создаётся объект `DatabaseManager("data/database.json")`.
Затем выводится меню:

| Log in (l)
| Create an account (c)
| Forgot password (f)
| Exit (e)


---

## 2. Создание клиента
1. Ввод имени, возраста, email, ID и секретного слова.  
2. Проверка ID (только цифры, длина 8–10).  
3. Если ID уже существует — переход в логин.  
4. Выбор количества счетов:  
   - `1 → Checking`;  
   - `2 → Checking + Savings (ставка 15%)`.  
5. Данные сохраняются в JSON.

---

## 3. Вход
1. Ввод ID и секретного слова.  
2. Проверка через `DatabaseManager::loadCustomer`.  
3. При успешном входе начисляются проценты по Savings.  
4. Клиент попадает в личное меню.

---

## 4. Меню клиента

    Deposit (ATM)

    Withdraw

    Create time deposit (Savings only)

    Transfer between accounts (only from Checking)

    Logout


- **Deposit** — ввод номинала и количества купюр, проверка.
- **Withdraw** — снятие средств с проверкой баланса.
- **Transfer** — перевод по `accId` между любыми клиентами.
- **Time deposit** — имитация вклада (уменьшает баланс Savings).
- **Logout** — сохранение клиента и выход в главное меню.

---

## 5. Забыл пароль
1. Ввод ID и email.  
2. Проверка совпадения.  
3. Установка нового `secretWord`.  
4. Сохранение в JSON.

---

## 6. Начисление процентов
При логине программа проверяет:
- если `type == "Savings"`;
- если прошли дни с момента `lastSavedDate`;
- начисляет простые проценты:  
  `interest = balance * rate * (days / 365.0)`;
- обновляет `lastSavedDate`.

