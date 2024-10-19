Two different types of parameters! *BOTH* saved/loaded as patch/song
* params[] <= audio thread owner, midi cc etc
* settings[] <= web server owner, creating objects etc.

PARAMS: run-time parameters
* Midi CC
* Deterministic (may not allocate memory)
* min/max log etc..
* recordable


SETTINGS: non run-time (settings?)
* string values
* may create objects
* we set data, then act on data..
* may be proceeded with racks (or units) being off-line
* yeah a unit must be taken off-line before it can be deleted..
online = true/false

rack {
  setttings {
    'synth': 'subreal'

  }
}

omg. we really must go from down to top..

unit {
    "type": "synth", (/?)
    "settings": {
        "class": "subreal",
        "name:"Master blaster", /?? filename?
        "polyfony": "12",
        "kbdCenter": "C4"
    }
    "params": {
        "cutoff": 125,
        "resonance": 101,
        "vca_attack": 10
    },
}

rack {
    type: "rack", /??
    name: "PolyDelay",
    eventor1: {}
    eventor2: {}
    synth: {
        type:"synth"
    }
    effect1: {
        settings: {
            class:"chorus"
        },
        params: {
            vca_attack:40
        }
    }
}