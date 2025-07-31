# C++ Style Guide: GEMINI Standard

This document outlines the standard coding conventions for C++ projects following the GEMINI standard. Consistency in code is crucial for readability and maintainability.

---

## Functions

### 1. Naming Convention

All function names must use **`lowerCamelCase`**. In this style, the first word is in all lowercase, and the first letter of each subsequent word is capitalized.

* **Rationale:** This convention is widely used and improves readability by making it easy to distinguish words within a function name without using separators.

* **Example:**

    ```cpp
    // CORRECT
    void printUserData();
    int calculateFinalScore();

    // INCORRECT
    void PrintUserData(); // PascalCase
    void print_user_data(); // snake_case
    ```

### 2. Documentation Brief

Every function declaration or definition must be preceded by a brief, single-line comment that clearly and concisely describes its purpose. The brief should explain *what* the function does, not *how* it does it.

* **Rationale:** A brief provides an immediate understanding of the function's responsibility, saving developers time from having to read the entire implementation to understand its purpose.

* **Example:**

    ```cpp
    /**
     * @brief Calculates the total price including tax and shipping.
     */
    double calculateTotalPrice(double subtotal);

    /**
     * @brief Connects the application to the primary database.
     */
    bool connectToDatabase(const std::string& connectionString);
    ```

---

## Comments

### 1. General Implementation Comments

All non-documentation comments used for explaining implementation details or separating logical blocks of code within a function must use the `// --- comment text ---` style.

* **Rationale:** This distinctive style makes comments stand out from the code and provides a clear visual separation, improving code scannability. It differentiates general remarks from formal documentation briefs.

* **Example:**

    ```cpp
    void processData() {
        // --- Sanitize the user input first ---
        sanitizeInput();

        // --- Now, perform the main calculation ---
        performCalculation();
    }
    ```

---
*This style guide should be adhered to for all new and refactored code.*
