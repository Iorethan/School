#!/bin/sh

make

echo "Start server, server listens at port 12312"
./server -p 12312 &

echo "Test upload in same directory"
./client -p 12312 -h localhost -u test.txt

echo "Diff after upload:"
diff test.txt ref_test.txt

echo "Test download in same directory"
./client -p 12312 -h localhost -d test.txt

echo "Diff after download:"
diff test.txt ref_test.txt

echo "Kill server"
kill -9 `ps | grep "./server" | grep -v "grep" | cut -f1 -d" "` 2>/dev/null