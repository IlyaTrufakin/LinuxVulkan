#include <iostream>
#include <cmath>

class QuadraticEquationsSolver {
public:
    /** \brief Calculates the discriminant of a quadratic equation.
     *
     *  This function calculates the discriminant of a quadratic equation of the form ax^2 + bx + c.
     *
     *  \param a The coefficient of x^2.
     *  \param b The coefficient of x.
     *  \param c The constant term.
     *  \return The value of the discriminant, which is equal to b^2 - 4ac.
     */
    static double discriminant(double a, double b, double c) {
        return b * b - 4 * a * c;
    }

    /** \brief Solves a quadratic equation of the form ax^2 + bx + c = 0.
     *  \param a The coefficient of x^2.
     *  \param b The coefficient of x.
     *  \param c The constant term.
     *  \return void This function does not return a value, it prints the roots of the equation (if any) to the console.
     *
     *  This function calculates the discriminant of the quadratic equation and determines the number of solutions.
     *  If the discriminant is negative, it prints "No roots".
     *  If the discriminant is positive, it calculates and prints the two real roots.
     *  If the discriminant is zero, it calculates and prints the single real root.
     */
    static void solve(double a, double b, double c) {
        double d = discriminant(a, b, c);
        if (d < 0) {
            std::cout << "No roots\n";
        } else if (d > 0) {
            double x1 = (-b + sqrt(d)) / (2.0 * a);
            double x2 = (-b - sqrt(d)) / (2.0 * a);
            std::cout << "x1 = " << x1 << ", x2 = " << x2 << "\n";
        } else {
            std::cout << "x = " << (-b) / (2.0 * a) << "\n";
        }
    }
};