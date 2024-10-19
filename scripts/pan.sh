if [ "$1" == "" ] ; then 
	curl 'http://0.0.0.0:18080/setRackUnitParam?rack=0&unit=synth&name=pan&value=53'
else
	curl 'http://0.0.0.0:18080/setRackUnitParam?rack=0&unit=synth&name=pan&value='${1}		
fi
