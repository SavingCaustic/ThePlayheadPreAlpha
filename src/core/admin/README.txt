here, the AdminQueue-reader is defined as a class and struct.

any actions (initated by api), is delegated here through a queue.
Some actions require interaction with player-engine such as taking racks off-line before managing them.

Player engine checks at end if a rack is requested to be taken of line
On next round of render, it checks in beginning if any request to bring rack on-line
Small admin-actions could possibly pass without cracking.

Not sure if static base class SynthUtility should be here or in the synth-directory..
Could make sense to keep it here. yeah it's both ways..
  + static
  - unit-admin interacts
