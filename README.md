- mobilenet SSD object detection on ncnn
```
mkdir build
cd build
cmake -DANDROID=1 ..
make
cp ssdmobilenet/ssdmobilenet ../ssdmobilenet/
cd ../ssdmobilenet
./ssdmobilenet test.jpg
```

* enjoy!



