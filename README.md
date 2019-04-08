# ToolsVisualisation
This repository contains tools and scripts to visualize and analyze intermediate results of the Processor-CPP pipeline

The usage of the program is the following:

./ToolsVisualisation-CPP config_file.txt

Where config_file.txt is a text file with the following parameters.

```
# Lines starting with # will be ommitted.
# Path to the folder with equirectnagular images
PATH_TO_EQUI =path_to_output_folder_of_converted_equirectangular_images
# Path to the folder with the models
PATH_TO_MODEL =path_to_the_models
# To show the matches between pair of images, use the parameter FILE_MATCHES2, otherwise, to show matches of triplets use the parameter FILE_MATCHES3
# File name of the file with the pair of matches
#FILE_MATCHES2 = 20190319-103906-094891_20190319-103906-344895
# File name of the file with the triplets of matches
FILE_MATCHES3 = 20190319-103949-594899_20190319-103949-844896_20190319-103950-094893
```
