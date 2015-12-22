(function(dataSet){
    /* Get values from first channel */
    var values = dataSet.getData(0);
    var sum = 0.0;
    var diff = 0.0;

    /* Read samples sequentially and compute sum */
    for(i = 0;i < values.length();++i){
        sum += values.getDoubleValue(i);
    }
    sum = sum / values.length();

    /* Compute spread */
    for(i = 0;i < values.length();++i){
        var x = values.getDoubleValue(i) - sum;
        diff += x*x;
    }
    diff = Math.sqrt(diff) / values.length();

    /* Return values we want to display */
    return [{"name": "Sum", "value": sum},
        {"name": "Spread", "value": diff}];
})
