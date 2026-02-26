# Формат базы данных (data/database.json)

Пример одной записи:

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

Поля клиента
Поле          Тип    Описание
name          string    Имя клиента
age           int    Возраст
email         string    Почта
secretWord    string    Пароль
accounts      array    Массив счетов клиента

Поля счёта
Поле             Тип    Описание
accId            int    Уникальный ID счёта
type             string    Checking / Savings
balance          double    Баланс
savingsRate      double    Процентная ставка
lastSavedDate    string    Дата последнего начисления процентов
