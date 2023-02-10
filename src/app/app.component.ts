import { Component, OnInit } from '@angular/core';
import { AngularFireDatabase, AngularFireObject } from '@angular/fire/compat/database';
import { Observable } from 'rxjs';
import { ESP32_DATA } from 'src/models/models';
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})


export class AppComponent implements OnInit {
  title = 'projet';
  private fichierRef: AngularFireObject<ESP32_DATA>;
  public fichier: Observable<any>;

  
  constructor(db: AngularFireDatabase) {
    this.fichierRef = db.object('ESP32_DATA');
    this.fichier = this.fichierRef.valueChanges();
   
  }
  ngOnInit(): void{
    this.fichierRef.snapshotChanges().subscribe((data) => {
      if (data.payload.exists()) {
        console.log(data.payload.val());
      } else {
        console.log("Node does not exist or there is no data in the node.");
      }
    })
  }
}


