import subprocess
import matplotlib.pyplot as plt

pathApp = r'cmake-build-debug/bin/nevk'
pathMesh = r'misc/bathroom/LAZIENKA.gltf'

framesDelay = 90
framesReport = 10

x = []  # resoulution
y = []  # msPF

'''
nevk [MODEL PATH] [MTL PATH] [OPTION...] positional parameters

  -m, --mesh arg     mesh path (default: "")
  -t, --texture arg  texture path (default: misc/)
      --width arg    window width (default: 800)
      --height arg   window height (default: 600)
  -h, --help         Print usage
'''

def collectData(width, height):
    global x
    global y

    process = subprocess.Popen(
        [pathApp, pathMesh, "--width=" + str(width), "--height=" + str(height), "--perfTest=" + "true",
         "--framesDelay=" + str(framesDelay), "--framesReport=" + str(framesReport)],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()

    data = list(map(float, stdout.split()))
    avMs = sum(data) / len(data)

    x.append(str(width) + 'x' + str(height))
    y.append(avMs)

# store data
width = [640, 800, 960, 1440]
height = [480, 600, 720, 1080]

for i in range(len(width)):
    collectData(width[i], height[i])

# create graph
plt.figure(figsize=(12, 7))
plt.plot(x, y, 'o-r', alpha=0.7, lw=5)
plt.xlabel('screen resolution')
plt.ylabel('msPF')
plt.savefig('perf.png')
