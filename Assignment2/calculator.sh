#!/bin/bash

while true
do
    echo "===== Calculator Menu ====="
    echo "1. Area of Circle"
    echo "2. Circumference of Circle"
    echo "3. Area of Rectangle"
    echo "4. Area of Square"
    echo "5. Exit"

    read choice

    case $choice in
    1)
        echo "Enter radius:"
        read r
        area=$(echo "3.14 * $r * $r" | bc)
        echo "Area = $area"
        ;;
    2)
        echo "Enter radius:"
        read r
        circ=$(echo "2 * 3.14 * $r" | bc)
        echo "Circumference = $circ"
        ;;
    3)
        echo "Enter length:"
        read l
        echo "Enter breadth:"
        read b
        echo "Area = $((l * b))"
        ;;
    4)
        echo "Enter side:"
        read s
        echo "Area = $((s * s))"
        ;;
    5)
        exit
        ;;
    *)
        echo "Invalid choice"
        ;;
    esac
done
