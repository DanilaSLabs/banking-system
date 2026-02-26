# Plans and development of the Banking System

## 🔒 Security
- Implement **password hashing (secretWord)** using SHA-256 or bcrypt.
- Add protection against ID matching (restriction of login attempts).

## 🖥 Interface
- Implement **a graphical interface (GUI)** on Qt or Dear ImGui.
- Add dark/light themes and visual notifications.
- Split the application into client and server parts.

## 💳 New features
- Support for **Credit Accounts** with a limit and interest.
- The possibility of opening **fixed-term deposits**.
- Maintaining transaction history in a separate JSON (`transactions.json`).
- Export the history to CSV/Excel.
- Add **Multilingual support (Russian/English interface)**.

## ⚙️ Infrastructure
- Add the **Unit Test module (GoogleTest)**.
- Automatic CI/CD build (GitHub Actions).
- Division into modules (libbankcore + UI).

---

📅 **Release plan:**
| Version | Main change |
|---------|--------------------|
| 1.1 | Hashing passwords |
| 1.2 | Graphical interface |
| 1.3 | Credit accounts |
| 1.4 | Transaction History |
| 2.0 | Full GUI with Network |
