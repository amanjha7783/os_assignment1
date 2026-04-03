#!/bin/bash

file="addressbook.txt"

while true
do
    echo "1. Create Address Book"
    echo "2. View Address Book"
    echo "3. Insert Record"
    echo "4. Delete Record"
    echo "5. Modify Record"
    echo "6. Exit"

    read choice

    case $choice in

    1)
        > $file
        echo "Address Book Created"
        ;;

    2)
        cat $file
        ;;

    3)
        echo "Enter Name:"
        read name
        echo "Enter Phone:"
        read phone
        echo "$name $phone" >> $file
        ;;

    4)
        echo "Enter name to delete:"
        read name
        grep -v "$name" $file > temp && mv temp $file
        ;;

    5)
        echo "Enter name to modify:"
        read name
        grep -v "$name" $file > temp
        echo "Enter new phone:"
        read phone
        echo "$name $phone" >> temp
        mv temp $file
        ;;

    6)
        exit
        ;;

    *)
        echo "Invalid choice"
        ;;

    esac
done

