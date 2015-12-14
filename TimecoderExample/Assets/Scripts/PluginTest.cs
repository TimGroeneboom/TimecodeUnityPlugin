using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

public class PluginTest : MonoBehaviour
{
    [DllImport("ASimplePlugin")]
    private static extern string PrintHello();

    void Start()
    {
        PrintHello();
    }
}
