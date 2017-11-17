object ChoosDlg: TChoosDlg
  Left = 227
  Top = 108
  BorderStyle = bsDialog
  Caption = #1055#1088#1077#1076#1091#1087#1088#1077#1078#1076#1077#1085#1080#1077
  ClientHeight = 113
  ClientWidth = 384
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 368
    Height = 51
    Shape = bsFrame
  end
  object Label1: TLabel
    Left = 70
    Top = 17
    Width = 241
    Height = 18
    Caption = #1059#1082#1072#1079#1072#1085#1085#1099#1081' '#1092#1072#1081#1083' '#1091#1078#1077' '#1089#1091#1097#1077#1089#1090#1074#1091#1077#1090'!'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 159
    Top = 36
    Width = 58
    Height = 18
    Caption = #1053#1086#1084#1077#1088#1072
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object chooseSigs: TEdit
    Left = 25
    Top = 65
    Width = 330
    Height = 21
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 2
    Visible = False
    OnKeyPress = chooseSigsKeyPress
  end
  object sigsInGrp: TEdit
    Left = 125
    Top = 50
    Width = 121
    Height = 21
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 3
    Visible = False
    OnKeyPress = sigsInGrpKeyPress
  end
  object AppEnd: TButton
    Left = 217
    Top = 77
    Width = 65
    Height = 25
    Caption = #1076#1086#1087#1080#1089#1072#1090#1100
    ModalResult = 6
    TabOrder = 4
  end
  object CancelBtn: TButton
    Left = 301
    Top = 77
    Width = 75
    Height = 25
    Caption = #1086#1090#1084#1077#1085#1080#1090#1100
    ModalResult = 2
    TabOrder = 5
  end
  object OKBtn: TButton
    Left = 8
    Top = 77
    Width = 105
    Height = 25
    Caption = #1074#1099#1073#1088#1072#1090#1100' '#1076#1088#1091#1075#1086#1081
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object OverWrite: TButton
    Left = 131
    Top = 77
    Width = 68
    Height = 25
    Cancel = True
    Caption = #1079#1072#1084#1077#1085#1080#1090#1100
    ModalResult = 5
    TabOrder = 1
  end
  object UnderDL: TCheckBox
    Left = 106
    Top = 92
    Width = 111
    Height = 17
    Caption = #1053#1091#1083#1077#1074#1099#1077' '#1089#1080#1075#1085#1072#1083#1099
    TabOrder = 6
    OnClick = UnderDLClick
  end
end
