# Cobalt Strike "Where Am I?" Beacon Object File
Cobalt Strike Beacon Object File (BOF) that outputs all the Environment strings without using DLL's.

See my [Creating the "Where Am I" Cobalt Strike BOF](https://0xboku.com/2021/08/19/Bof-WhereAmI.html) blog post for a walkthrough on how I created this Cobalt Strike Beacon Object File from start to finish.

![](/images/beaconLoop.png)

![](/images/greatSuccess.png)

### BOF Flow to get the Environment Variables Dynamically in Memory
TEB (GS Register) --> PEB --> ProcessParameters --> Environment

![](/images/bangPEB.png)

