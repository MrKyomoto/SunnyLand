# Tiled Map Parser
- get layers[] and iterates it
  - if type : imagelayer, load single image
    - pay attention to "parallax", "repeat", "offset"
    - create game object containing ParallaxComponent
  - if type : tilelayer ...
  - if type : objectgroup ...
