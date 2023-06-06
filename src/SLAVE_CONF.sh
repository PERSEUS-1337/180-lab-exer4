
#!/bin/bash
# Change to the 'exe' directory
cd ./exe || exit

start_port=6000
num_slaves=16
matx_size=8000

# Loop to create the files
for ((i=1; i<=$num_slaves; i++))
do
    # Calculate the current port number
    port=$((start_port + i - 1))

    # Create the file with the appropriate name
    filename="SLAVE_$i.in"

    echo "1" > "$filename"

    # Write the port number to the file
    echo "$port" >> "$filename"

    echo "Created file $filename with port $port"
done

# Write the MASTER.in file
echo "2" > MASTER.in
echo "$num_slaves" >> MASTER.in
echo "$matx_size" >> MASTER.in

# Loop to write the IP addresses and port numbers
for ((i=0; i<$num_slaves; i++))
do
    # Calculate the current port number
    port=$((start_port + i))

    # Write the IP address and port number
    echo "127.0.1.1" >> MASTER.in
    echo "$port" >> MASTER.in
done

echo "Created file MASTER.in with specified contents."