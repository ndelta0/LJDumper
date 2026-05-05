function Component() {
}

Component.prototype.createOperations = function() {
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation(
            "CreateShortcut",
            "@TargetDir@/bin/LJDumper.exe",
            "@StartMenuDir@/LJDumper.lnk",
            "workingDirectory=@TargetDir@/bin",
            "description=LJDumper"
        );

        component.addOperation(
            "CreateShortcut",
            "@TargetDir@/bin/LJDumper.exe",
            "@DesktopDir@/LJDumper.lnk",
            "workingDirectory=@TargetDir@/bin",
            "description=LJDumper"
        );
    }
}