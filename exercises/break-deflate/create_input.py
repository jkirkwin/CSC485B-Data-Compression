import sys
import os

DEFAULT_FILENAME = 'broken.bin'
GZIP_BUF_SIZE = 32768
REPEAT_SIZE = GZIP_BUF_SIZE + 1
BZIP_BUF_SIZE = 900000 # In reality it is slightly larger

def main(filename):
    pattern = os.urandom(REPEAT_SIZE)
    bytes_written = 0
    with open(filename, 'wb') as f:
        while(bytes_written + REPEAT_SIZE <= BZIP_BUF_SIZE):
            f.write(pattern)
            bytes_written += REPEAT_SIZE
        remaining = BZIP_BUF_SIZE - bytes_written
        f.write(pattern[:remaining])

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_FILENAME 
    main(filename)