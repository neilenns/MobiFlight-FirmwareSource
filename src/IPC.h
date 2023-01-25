#pragma once

volatile extern double values[8];

enum ChangedValue {
    Com1Active,
    Com2Active,
    Com1Standby,
    Com2Standby,
    Nav1Active,
    Nav2Active,
    Nav1Standby,
    Nav2Standby
};