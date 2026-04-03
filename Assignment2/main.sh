#!/bin/bash

while true
do
    echo "===== MAIN MENU ====="
    echo "1. Factorial"
    echo "2. Address Book"
    echo "3. Rename JPG Files"
    echo "4. Calculator"
    echo "5. Exit"

    read choice

    case $choice in
    1)
        bash factorial.sh
        ;;
    2)
        bash addressbook.sh
        ;;
    3)
        bash renamejpg.sh
        ;;
    4)
        bash calculator.sh
        ;;
    5)
        echo "Exiting..."
        exit
        ;;
    *)
        echo "Invalid choice"
        ;;
    esac
done
