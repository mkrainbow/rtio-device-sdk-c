# Copyright (c) 2024-2025 mkrainbow.com.
#
# Licensed under MIT.
# See the LICENSE for detail or copy at https://opensource.org/license/MIT.

import json  
import argparse
import crc

def printRed(text):
    print('\033[91m' + text + '\033[0m')   
  
def process_file(filename):  
    
    calcu = crc.Calculator(crc.Crc32.CRC32)
    # Initialize an empty dictionary to store digests  
    digest_map = {}  
    dup = 0
    total = 0
    err = 0
  
    # Open the file and read it line by line  
    with open(filename, 'r') as file:  
        for line in file:  
            # Remove trailing newline character (if present)  
            line = line.strip()  
            total = total + 1
              
            # Try to parse the JSON string  
            try:  
                data = json.loads(line)  
                  
                # Extract the uri and digest fields  
                uri = data['uri']  
                digest = data['digest']  

                # Check crc
                digest2 = calcu.checksum(uri.encode("ascii"))
                if digest2 != int(digest):
                    err = err + 1
                    printRed(f"ERR #{err} URI:{uri}, Digest: {digest} != {digest2}") 
                
                # Check if the digest already exists in the map  
                if digest in digest_map: 
                    dup = dup + 1
                    printRed(f"DUP #{dup} URI: {uri}, Digest: {digest}") 
                else:  
                    # Add the digest to the map  
                    digest_map[digest] = uri  
                      
                # Optionally, you can print the uri and digest for verification  
                # print(f"URI: {uri}, Digest: {digest}")  
                  
            except json.JSONDecodeError as e:  
                # If the line is not a valid JSON, print an error message  
                printRed(f"Error parsing JSON on line: {line}. Error: {e}")  
  
    # Output the end message  
    printRed(f"Check: total={total}, dup={dup}, err={err}.")  
  
    # Optionally, you can print the final digest map if needed  
    # print("Digest Map:")  
    # for digest, uri in digest_map.items():  
    #     print(f"Digest: {digest}, URI: {uri}")  
  
if __name__ == "__main__":  
    # Create the argument parser  
    parser = argparse.ArgumentParser(description="Process URIs and digests from a file.")  
      
    # Add an argument for the filename  
    parser.add_argument("filename", type=str, help="The file containing the URIs and digests.")  
      
    # Parse the arguments  
    args = parser.parse_args()  
      
    # Call the function with the provided filename  
    process_file(args.filename)